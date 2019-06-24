// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// File: embeddingapi.cpp
//

#include "common.h"

void *embedding_api_callbacksptr = NULL;
bool embedding_api_handle_allocated = false;
dotnet_threadstarted embedding_api_thread_started = NULL;
dotnet_threadstopped embedding_api_thread_stopped = NULL;
dotnet_gc_event embedding_api_gc_event = NULL;

dotnet_error embeddingapi_impl_writemanagedmem(CorElementType et, TypeHandle th, uint8_t* pPointerToManagedMemory, uint8_t* pUnmanagedData, int32_t cbDataUnmanaged);
dotnet_error embeddingapi_impl_readmanagedmem(dotnet_frame frame, CorElementType et, TypeHandle th, uint8_t* pPointerToManagedMemory, uint8_t* pUnmanagedData, int32_t cbDataUnmanaged);

#define ENTER_EMBEDDING_API CONTRACTL \
    { \
        THROWS; \
        GC_TRIGGERS; \
        MODE_PREEMPTIVE; \
        EE_THREAD_NOT_REQUIRED; \
    } \
    CONTRACTL_END; \
    dotnet_error result = 0; \
    BEGIN_GETTHREAD_ALLOWED; \
    Thread *pThread = GetThreadNULLOk(); \
    if (pThread == NULL) \
        pThread = SetupThread(); \
    EX_TRY \
    { \


#define END_EMBEDDING_API } EX_CATCH_HRESULT_NO_ERRORINFO (result); END_GETTHREAD_ALLOWED; return result

enum class GetApiHelperEnum
{
    // Keep this in sync with the num in embeddingapi_impl.h
    object_tostring,
    type_gettype,
    type_getmethod,
    type_getfield,
    type_get_element_type,
    type_getconstructor,
    string_alloc_utf8,
    utf8_getstring,
    get_method_from_methodid
};

static void* GetApiFromManaged(GetApiHelperEnum api);


// Utility functions
dotnet_error embeddingapi_alloc(uint32_t cb, void ** bytes)
{
    STANDARD_VM_CONTRACT;
    *bytes = CoTaskMemAlloc(cb);
    if (*bytes == NULL)
        return E_OUTOFMEMORY;
    else
        return S_OK;
}

dotnet_error embeddingapi_free(void * bytes)
{
    STANDARD_VM_CONTRACT;
    CoTaskMemFree(bytes);
    return S_OK;
}

// thread local handle functionality

ExpandoEmbeddingApiFrame::ExpandoEmbeddingApiFrame() : _expando(NULL), _remainingFreeElements(s_elementCount)
{
}

/*static*/ bool ExpandoEmbeddingApiFrame::AllocEntry(ExpandoEmbeddingApiFrame** ppFrame, void *newValue, void **allocatedEntry)
{
    ExpandoEmbeddingApiFrame* pFrame = *ppFrame;
    if (pFrame->_remainingFreeElements > 0)
    {
        pFrame->_remainingFreeElements = pFrame->_remainingFreeElements - 1;
        *allocatedEntry = &pFrame->_elements[pFrame->_remainingFreeElements];
    }
    else
    {
        ExpandoEmbeddingApiFrame* pNewFrame = new(nothrow) ExpandoEmbeddingApiFrame;
        if (pNewFrame == NULL)
            return false;

        pNewFrame->_remainingFreeElements = s_elementCount - 1;
        *allocatedEntry = &pNewFrame->_elements[s_elementCount - 1];
        pFrame->_expando = pNewFrame;
        *ppFrame = pNewFrame;
    }

    **(void***)allocatedEntry = newValue;
    return true;
}

void ExpandoEmbeddingApiFrame::GCPromote(promote_func* fn, ScanContext* sc)
{
    for (unsigned i = 16; i > _remainingFreeElements; i--)
    {
        UINT_PTR ptrInFrame = (UINT_PTR)_elements[i - 1];
        switch (ptrInFrame & 0x3)
        {
        case 0:
            fn((Object**)&_elements[i - 1], sc, 0);
            break;
        case 1:
            fn((Object**)&_elements[i - 1], sc, GC_CALL_INTERIOR | GC_CALL_PINNED);
            break;

        default:
            // Do nothing, these are in the freelist
            break;
        }
    }
}

ExpandoEmbeddingApiFrame::~ExpandoEmbeddingApiFrame()
{
    delete _expando;
}

FrameForEmbeddingApi::FrameForEmbeddingApi(Thread *thread) :_expando(&_initialFrame), _nextFreeElement(NULL), _thread(thread), _next(thread->m_pEmbeddingApiFrame)
{
    thread->m_pEmbeddingApiFrame = this;
}

void FrameForEmbeddingApi::GCPromote(promote_func* fn, ScanContext* sc)
{
    ExpandoEmbeddingApiFrame *pCurrentExpando = &_initialFrame;
    while (pCurrentExpando != NULL)
    {
        pCurrentExpando->GCPromote(fn, sc);
        pCurrentExpando = pCurrentExpando->_expando;
    }
}

bool FrameForEmbeddingApi::AllocEntry(void *newValue, void **allocatedEntry)
{
    if (_nextFreeElement != NULL)
    {
        void **oldFreeElement = (void**)_nextFreeElement;
        _nextFreeElement = (FreeElement*)(_nextFreeElement->_nextFreeElement - 2);
        *oldFreeElement = newValue;
        *allocatedEntry = oldFreeElement;
        return true;
    }
    else
    {
        return ExpandoEmbeddingApiFrame::AllocEntry(&_expando, newValue, allocatedEntry);
    }
}

void FrameForEmbeddingApi::FreeEntry(void *entryToFree)
{
    _ASSERTE(entryToFree != NULL);
    _ASSERTE((((UINT_PTR)entryToFree) & 0x3) == 0);
    FreeElement *newFreeElement = (FreeElement*)entryToFree;
    newFreeElement->_nextFreeElement = ((UINT_PTR)_nextFreeElement) + 2;
    _nextFreeElement = newFreeElement;
}

bool FrameForEmbeddingApi::Pop()
{
    if (_thread->m_pEmbeddingApiFrame == this)
    {
        delete this;
        return true;
    }
    else
    {
        return false;
    }
}

FrameForEmbeddingApi::~FrameForEmbeddingApi()
{
    _thread->m_pEmbeddingApiFrame = this->_next;
}

dotnet_error embeddingapi_push_frame(dotnet_frame *frame)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();
    *frame = (dotnet_frame)new(nothrow) FrameForEmbeddingApi(pThread);

    result = (*frame) == NULL ? E_OUTOFMEMORY : S_OK;

    END_EMBEDDING_API;
}

FCIMPL0(dotnet_frame, EmbeddingApi::nPushFrame)
{
    FCALL_CONTRACT;
    dotnet_frame newFrame = (dotnet_frame)new(nothrow) FrameForEmbeddingApi(GetThread());
    if (newFrame != NULL)
        return newFrame;
    else
        FCThrow(kOutOfMemoryException);
}
FCIMPLEND;

dotnet_error embeddingapi_push_frame_collect_on_return(dotnet_frame *frame)
{
    // This isn't implemented yet
    return E_NOTIMPL;
}

dotnet_error embeddingapi_pop_frame(dotnet_frame frame)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    result = actualFrame->Pop() ? S_OK : E_INVALIDARG;

    END_EMBEDDING_API;
}

FCIMPL1(void, EmbeddingApi::nPopFrame, dotnet_frame frame)
{
    FCALL_CONTRACT;
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    if (actualFrame->Pop())
        return;
    else
    {
        FCThrowVoid(kInvalidOperationException);
    }
}
FCIMPLEND;

dotnet_error embeddingapi_handle_free(dotnet_frame frame, dotnet_object object)
{
    ENTER_EMBEDDING_API;

    if (object != NULL)
    {
        GCX_COOP();
        FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
        actualFrame->FreeEntry(object);
    }

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_handle_alloc(dotnet_frame frame, OBJECTREF objRef, dotnet_object *handle)
{
    // THIS ISN'T public api surface, but it is used by logic which produces new embedding handles
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_COOPERATIVE;
        EE_THREAD_REQUIRED;
    }
    CONTRACTL_END;

    if (objRef == NULL)
        return NULL;
    
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    if (actualFrame->AllocEntry(objRef->GetAddress(), (void**)handle))
        return S_OK;
    else
        return E_OUTOFMEMORY;
}

FCIMPL2(dotnet_object, EmbeddingApi::nAllocHandle, dotnet_frame frame, Object* objectUNSAFE)
{
    FCALL_CONTRACT;

    if (objectUNSAFE == NULL)
        return NULL;

    OBJECTREF objRef = ObjectToOBJECTREF(objectUNSAFE);
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    dotnet_object retVal;
    if (!actualFrame->AllocEntry(objRef->GetAddress(), (void**)&retVal))
        FCThrow(kOutOfMemoryException);

    return retVal;
}
FCIMPLEND;

OBJECTREF embeddingapi_get_target(dotnet_object obj)
{
    if (obj == NULL)
        return NULL;

    return ObjectToOBJECTREF(*(Object**)obj);
}

FCIMPL1(Object*, EmbeddingApi::nGetTarget, dotnet_object obj)
{
    return OBJECTREFToObject(embeddingapi_get_target(obj));
}
FCIMPLEND;

dotnet_error embeddingapi_handle_pin(dotnet_frame frame, dotnet_object object, dotnet_pin* pin, void**data)
{
    ENTER_EMBEDDING_API;

    if (object == NULL)
        result = E_INVALIDARG;
    else
    {
        GCX_COOP();
        FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;

        OBJECTREF objRef = ObjectToOBJECTREF(*(Object **)object);
        MethodTable *pMT = objRef->GetMethodTable();

        if (!pMT->IsPinnable())
        {
            result = E_INVALIDARG;
        }
        else
        {
            if (pMT->HasComponentSize())
            {
                *data = objRef->GetAddress() + ArrayBase::GetDataPtrOffset(pMT);
            }
            else
            {
                *data = objRef->GetData();
            }
        }

        if (actualFrame->AllocEntry(objRef->GetAddress() + 1, (void**)pin)) // The +1 provides information to the GC walker in the frame to indicate this is a pinned reference.
            result = S_OK;
        else
            result = E_OUTOFMEMORY;
    }

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_handle_unpin(dotnet_frame frame, dotnet_pin pin)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    actualFrame->FreeEntry(pin);

    END_EMBEDDING_API;
}

// object api surface
dotnet_error embeddingapi_object_gettypeid(dotnet_object obj, dotnet_typeid* type)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();
    OBJECTREF objRef = embeddingapi_get_target(obj);
    *type = (dotnet_typeid)objRef->GetTypeHandle().AsPtr();

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_object_isinst(dotnet_object obj, dotnet_typeid type, int32_t* isType)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    OBJECTREF objRef = embeddingapi_get_target(obj);
    *isType = (int32_t)ObjIsInstanceOf(OBJECTREFToObject(objRef), th, FALSE);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_object_alloc(dotnet_frame frame, dotnet_typeid type, dotnet_object* object)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    result = embeddingapi_handle_alloc(frame, AllocateObject(th.GetMethodTable()), object);

    END_EMBEDDING_API;
}

// gchandle api surface
dotnet_error embeddingapi_gchandle_alloc(dotnet_object obj, dotnet_gchandle *gchandle)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();
    *gchandle = NULL;
    OBJECTREF objRef = embeddingapi_get_target(obj);
    OBJECTHANDLE gch = CreateGlobalHandle(objRef);
    *gchandle = (dotnet_gchandle)gch;
    result = S_OK;

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_gchandle_alloc_weak(dotnet_object obj, dotnet_gchandle *gchandle)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();

    *gchandle = NULL;
    OBJECTREF objRef = embeddingapi_get_target(obj);
    OBJECTHANDLE gch = CreateGlobalShortWeakHandle(objRef);
    *gchandle = (dotnet_gchandle)gch;
    result = S_OK;

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_gchandle_alloc_weak_track_resurrection(dotnet_object obj, dotnet_gchandle *gchandle)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();

    *gchandle = NULL;
    OBJECTREF objRef = embeddingapi_get_target(obj);
    OBJECTHANDLE gch = CreateGlobalLongWeakHandle(objRef);
    *gchandle = (dotnet_gchandle)gch;

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_gchandle_free(dotnet_gchandle gchandle)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();

    OBJECTHANDLE gch = (OBJECTHANDLE)gchandle;
    DestroyTypedHandle(gch);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_gchandle_get_target(dotnet_frame frame, dotnet_gchandle gchandle, dotnet_object *object)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();

    OBJECTHANDLE gch = (OBJECTHANDLE)gchandle;
    OBJECTREF objRef = ObjectFromHandle(gch);
    result = embeddingapi_handle_alloc(frame, objRef, object);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_toggleref_creategroup(dotnet_togglerefcallback callback, dotnet_togglerefgroup *togglerefgroup)
{
    ENTER_EMBEDDING_API;

    GCHandleUtilities::GetGCHandleManager()->CreateHandleStore((ref_counted_handle_callback_func*)callback);

    END_EMBEDDING_API;
}


dotnet_error embeddingapi_toggleref_alloc(dotnet_togglerefgroup togglerefgroup, dotnet_object obj, dotnet_toggleref *gchandle)
{
    *gchandle = NULL;
    ENTER_EMBEDDING_API;

    GCX_COOP();

    OBJECTREF objRef = embeddingapi_get_target(obj);
    IGCHandleStore *store = (IGCHandleStore*)togglerefgroup;
    OBJECTHANDLE gch = CreateRefcountedHandle(store, objRef);
    *gchandle = (dotnet_toggleref)gch;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_toggleref_free(dotnet_togglerefgroup togglerefgroup, dotnet_toggleref gchandle)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();

    OBJECTHANDLE gch = (OBJECTHANDLE)gchandle;
    DestroyTypedHandle(gch);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_toggleref_get_target(dotnet_frame frame, dotnet_togglerefgroup togglerefgroup, dotnet_toggleref gchandle, dotnet_object *object)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();

    OBJECTHANDLE gch = (OBJECTHANDLE)gchandle;
    OBJECTREF objRef = ObjectFromHandle(gch);
    result = embeddingapi_handle_alloc(frame, objRef, object);

    END_EMBEDDING_API;
}

// Id access surface
dotnet_error embeddingapi_get_methodid(dotnet_object method, dotnet_methodid* methodid)
{
    if (method == NULL)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    GCX_COOP();
    REFLECTMETHODREF methodObj = (REFLECTMETHODREF)ObjectToOBJECTREF(*(Object**)method);
    MethodDesc *pMD = methodObj->GetMethod();
    *methodid = (dotnet_methodid)pMD;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_get_typeid(dotnet_object typeObject, dotnet_typeid* typeidVar)
{
    if (typeObject == NULL)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    GCX_COOP();
    REFLECTCLASSBASEREF typeObj = (REFLECTCLASSBASEREF)ObjectToOBJECTREF(*(Object**)typeObject);
    TypeHandle th = typeObj->GetType();
    *typeidVar = (dotnet_typeid)th.AsPtr();

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_get_fieldid(dotnet_object fieldObject, dotnet_fieldid* fieldidVar)
{
    if (fieldObject == NULL)
        return E_INVALIDARG;
    
    ENTER_EMBEDDING_API;

    GCX_COOP();
    REFLECTFIELDREF fieldObj = (REFLECTFIELDREF)ObjectToOBJECTREF(*(Object**)fieldObject);
    FieldDesc* fd = fieldObj->GetField();
    fd->LoadSize(); // CAN THROW, NEEDS PROTECTION
    fd->GetApproxFieldTypeHandleThrowing(); // CAN THROW, NEEDS PROTECTION

    *fieldidVar = (dotnet_fieldid)fd;

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_get_type_from_typeid(dotnet_frame frame, dotnet_typeid type, dotnet_object *type_dotnet_obj)
{
    ENTER_EMBEDDING_API;

    TypeHandle th = TypeHandle::FromPtr((void*)type);
    GCX_COOP();
    OBJECTREF typeObject = th.GetManagedClassObject();
    result = embeddingapi_handle_alloc(frame, typeObject, type_dotnet_obj);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_typeid_is_assignable_from(dotnet_typeid typeA, dotnet_typeid typeB, int32_t *resultBool)
{
    ENTER_EMBEDDING_API;
    TypeHandle thA = TypeHandle::FromPtr((void*)typeA);
    TypeHandle thB = TypeHandle::FromPtr((void*)typeB);
    *resultBool = thB.CanCastTo(thA) ? 1 : 0;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_typeid_is_valuetype(dotnet_typeid type, int32_t *resultBool)
{
    ENTER_EMBEDDING_API;
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    *resultBool = th.IsValueType() ? 1 : 0;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_typeid_field_size(dotnet_typeid type, int32_t *size)
{
    ENTER_EMBEDDING_API;
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    *size = (int32_t)th.GetSize();
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_typeid_is_enum(dotnet_typeid type, int32_t *resultBool)
{
    ENTER_EMBEDDING_API;
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    *resultBool = th.IsEnum() ? 1 : 0;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_typeid_is_class(dotnet_typeid type, int32_t *resultBool)
{
    ENTER_EMBEDDING_API;
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    *resultBool = !th.IsTypeDesc() && !th.GetMethodTable()->IsValueType() ? 1 : 0;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_typeid_enum_underlying_type(dotnet_typeid type, dotnet_typeid *underlyingType)
{
    *underlyingType = 0;
    ENTER_EMBEDDING_API;
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    if (!th.IsEnum())
    {
        result = E_INVALIDARG;
    }
    else
    {
        CorElementType underlyingTypeElementType = th.GetInternalCorElementType();
        TypeHandle th = TypeHandle(MscorlibBinder::GetElementType(underlyingTypeElementType));
        *underlyingType = (dotnet_typeid)th.AsPtr();
    }
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_typeid_is_byref(dotnet_typeid type, int32_t *resultBool)
{
    ENTER_EMBEDDING_API;
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    *resultBool = th.IsByRef() ? 1 : 0;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_methodid_get_signature(dotnet_methodid method, dotnet_methodsignature* sig)
{
    ENTER_EMBEDDING_API;
    MethodDesc *pMD = (MethodDesc*)method;
    MetaSig *pSig = new(nothrow)MetaSig(pMD);
    if (pSig == NULL)
    {
        result = E_OUTOFMEMORY;
        *sig = 0;
    }
    else
    {
        *sig = (dotnet_methodsignature)pSig;
        result = S_OK;
    }

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_methodsignature_free(dotnet_methodsignature sig)
{
    ENTER_EMBEDDING_API;
    delete (MetaSig*)sig;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_methodsignature_get_argument_count(dotnet_methodsignature sig, int32_t *argCount)
{
    ENTER_EMBEDDING_API;
    MetaSig *pSig = (MetaSig*)sig;
    *argCount = (int32_t)pSig->NumFixedArgs();
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_methodsignature_is_instance(dotnet_methodsignature sig, int32_t *resultBool)
{
    ENTER_EMBEDDING_API;
    MetaSig *pSig = (MetaSig*)sig;
    *resultBool = pSig->HasThis() ? 1 : 0;
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_methodsignature_get_nextarg_typeid(dotnet_methodsignature sig, dotnet_typeid *type)
{
    ENTER_EMBEDDING_API;
    MetaSig *pSig = (MetaSig*)sig;
    if (pSig->NextArg() == ELEMENT_TYPE_END)
    {
        result = E_INVALIDARG;
    }
    else
    {
        TypeHandle th = pSig->GetLastTypeHandleThrowing();
        *type = (dotnet_typeid)th.AsPtr();
    }

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_methodsignature_get_return_typeid(dotnet_methodsignature sig, dotnet_typeid *type)
{
    ENTER_EMBEDDING_API;
    MetaSig *pSig = (MetaSig*)sig;
    TypeHandle th = pSig->GetRetTypeHandleThrowing();
    *type = (dotnet_typeid)th.AsPtr();
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_box(dotnet_frame frame, dotnet_typeid type, void* pData, int32_t cbData, dotnet_object *boxedObject)
{
    ENTER_EMBEDDING_API;
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    if (!th.IsValueType())
    {
        result = E_INVALIDARG;
    }
    else if (th.IsByRefLike())
    {
        result = E_INVALIDARG;
    }
    else
    {
        if (Nullable::IsNullableType(th))
        {
            MethodTable *nullableMT = th.GetMethodTable();
            MethodTable *argMT = nullableMT->GetInstantiation()[0].GetMethodTable();
            CorElementType et = argMT->GetInternalCorElementType();

            GCX_COOP();
            OBJECTREF nonNullableObj = argMT->Allocate();
            Nullable* src = (Nullable*)pData;
            TypeHandle thArg(argMT);

            result = embeddingapi_impl_writemanagedmem(et, thArg, (uint8_t*)nonNullableObj->UnBox(), (uint8_t*)src->ValueAddr(nullableMT), argMT->GetNumInstanceFieldBytes());
        }
        else
        {
            CorElementType et = th.GetInternalCorElementType();
            GCX_COOP();
            OBJECTREF obj = AllocateObject(th.GetMethodTable());
            result = embeddingapi_impl_writemanagedmem(et, th, (uint8_t*)obj->UnBox(), (uint8_t*)pData, th.GetSize());
        }
    }

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_unbox(dotnet_frame frame, dotnet_object boxedObject, dotnet_typeid type, void* pData, int32_t cbData)
{
    ENTER_EMBEDDING_API;
    TypeHandle th = TypeHandle::FromPtr((void*)type);
    if (!th.IsValueType())
    {
        result = E_INVALIDARG;
    }
    else if (th.IsByRefLike())
    {
        result = E_INVALIDARG;
    }
    else
    {
        bool done = false;
        GCX_COOP();
        if (Nullable::IsNullableType(th))
        {
            if (boxedObject == NULL)
            {
                memset(pData, 0, cbData);
                done = true;
            }
            else
            {
                CLR_BOOL *pBool = ((Nullable*)pData)->HasValueAddr(th.GetMethodTable());
                *pBool = true;
                uint8_t* pValueData = (uint8_t*)((Nullable*)pData)->ValueAddr(th.GetMethodTable());
                cbData -= (int32_t)(pValueData - (uint8_t*)pData);
                pData = pValueData;
                th = th.GetMethodTable()->GetInstantiation()[0];
            }
        }
        if (!done)
        {
            TypeHandle thBoxedObject;
            {
                OBJECTREF boxedObjRef = embeddingapi_get_target(boxedObject);
                thBoxedObject = boxedObjRef->GetTypeHandle();
            }
            if (!thBoxedObject.IsEquivalentTo(th))
            {
                result = E_INVALIDARG;
            }
            else
            {
                CorElementType et = thBoxedObject.GetInternalCorElementType();
                OBJECTREF boxedObjRef = embeddingapi_get_target(boxedObject);
                result = embeddingapi_impl_readmanagedmem(frame, et, th, (uint8_t*)boxedObjRef->UnBox(), (uint8_t*)pData, cbData);
            }
        }
    }

    END_EMBEDDING_API;
}

// Method invoke implementation
dotnet_error embeddingapi_method_invoke(dotnet_frame frame, dotnet_methodid methodId, dotnet_invokeargument* arguments, int32_t countOfArguments, dotnet_methodinvoke_flags flags)
{
    ENTER_EMBEDDING_API;

    GCX_COOP();

    ARG_SLOT *pArgSlots = (ARG_SLOT*)_alloca(sizeof(ARG_SLOT) * countOfArguments - 1);

    MethodDescCallSite methodToCall((MethodDesc*)methodId);

    for (int i = 0; i < (countOfArguments - 1) && (result == S_OK); i++)
    {
        dotnet_invokeargument invokeArg = arguments[i+1];
        TypeHandle argumentTypeHandle = TypeHandle::FromPtr((void*)invokeArg.type);
        pArgSlots[i] = (ARG_SLOT)0;

        // 8 possible cases (NOTE: the concept of Primitive or enum value is the calling convention concept, not strictly the managed construct)
        // 1. Primitive or enum value. Encode directly in ARG_SLOT
        // 2. Reference type value. Encode translated value directly in ARG_SLOT
        // 3. Structure without gc pointers. Place pointer to structure in invokeargument directly into ARG_SLOT. (Works for byref and non-byref cases)
        // 4. Structure with gc pointers. Allocate new GC protected buffer of appropriate size, Copy data into it, but where copying GC pointers, perform translation. Place pointer to copied struct into ARG_SLOT
        // 5. Primitive or enum value BYREF. Place pointer to structure in invokeargument directly into ARG_SLOT.
        // 6. Reference type value BYREF. Allocate GC protected location for reference type. extract to there, once function call completes, marshal back as new BYREF
        // 7. Structure without gc pointers BYREF. As above for Structure without gc pointers exactly
        // 8. Structure with gc pointers BYREF. As above for Structure with gc pointers, but also copy out into original buffer

        // TODO!: The above rules are quite possibly inaccurate. Further investigation of the reflection invoke call path is needed
        // In particular, its known that this isn't how the this pointer must be marshalled for instance methods, and byref returns aren't handled correctly here

        // Current implementation covers clases 1,2, 3, 5 and 7
        CorElementType etCallingConvention = argumentTypeHandle.GetInternalCorElementType();

        if (CorTypeInfo::IsObjRef(etCallingConvention))
        {
            // Case 2.
            pArgSlots[i] = (ARG_SLOT)**(Object***)invokeArg.data;
        }
        else if (etCallingConvention == ELEMENT_TYPE_VALUETYPE)
        {
            if (!argumentTypeHandle.GetMethodTable()->ContainsPointers())
            {
                // Case 3
                pArgSlots[i] = (ARG_SLOT)invokeArg.data;
            }
            else
            {
                // Case 4
                result = E_FAIL;
            }
        }
        else if (etCallingConvention == ELEMENT_TYPE_BYREF)
        {
            TypeHandle thByRefOf = argumentTypeHandle.GetTypeParam();
            etCallingConvention = thByRefOf.GetInternalCorElementType();

            if (CorTypeInfo::IsObjRef(etCallingConvention))
            {
                // Case 6.
                result = E_FAIL;
            }
            if (!argumentTypeHandle.GetMethodTable()->ContainsPointers())
            {
                // Case 5 and 7
                pArgSlots[i] = (ARG_SLOT)invokeArg.data;
            }
            else
            {
                // Case 8
                result = E_FAIL;
            }
        }
        else
        {
            // Case 1
            _ASSERTE(argumentTypeHandle.GetSize() <= sizeof(ARG_SLOT));
            memcpy(&pArgSlots[i], invokeArg.data, argumentTypeHandle.GetSize());
        }
    }

    if (result == S_OK)
    {
        ARG_SLOT returnValue = methodToCall.Call_RetArgSlot(pArgSlots);

        // There are a variety of return paths...
        // Handle two of them

        TypeHandle returnTypeHandle = TypeHandle::FromPtr((void*)arguments[0].type);
        CorElementType etReturnType = returnTypeHandle.GetInternalCorElementType();

        if (CorTypeInfo::IsObjRef(etReturnType))
        {
            result = embeddingapi_handle_alloc(frame, ArgSlotToObj(returnValue), (dotnet_object*)arguments[0].data);
        }
        else if (etReturnType == ELEMENT_TYPE_VALUETYPE)
        {
            result = E_FAIL;
        }
        else
        {
            // Copy argslot contents back into data
            memcpy(arguments[0].data, &returnValue, returnTypeHandle.GetSize());
            result = S_OK;
        }
    }

    END_EMBEDDING_API;
}

template <class lambda_t>
bool enumerate_gc_references_in_field_layout(TypeHandle th, uint8_t* field_start, lambda_t exp)
{
    if (CorTypeInfo::IsObjRef(th.GetInternalCorElementType()))
    {
        return exp((uint8_t**)field_start);
    }
    else if (th.IsTypeDesc())
    {
        return true; // TypeDescs that are not object refs do not have gc pointers
    }

    MethodTable *pMT = th.GetMethodTable();

    if (!pMT->ContainsPointers())
    {
        return true; // Methodtable without gc pointers, does not need additional work
    }

    // This algorithm was originally built for the GC, and is actually designed to work with boxed valuetypes.
    // Since this particular api is actually working on unboxing valuetypes, adjust the pointer to make it work.
    uint8_t* o = field_start - sizeof(MethodTable*);
    size_t size = pMT->GetBaseSize();
    uint8_t* limit = o + size;

    CGCDesc* map = CGCDesc::GetCGCDescFromMT(pMT);
    CGCDescSeries* cur = map->GetHighestSeries();
    ptrdiff_t cnt = (ptrdiff_t)map->GetNumSeries();

    _ASSERTE(cnt >= 0); // Values less than 0 indicate strings and arrays, which were handled above as object ref cases
    CGCDescSeries* last = map->GetLowestSeries();
    uint8_t** parm = 0;
    do
    {
        assert(parm <= (uint8_t**)((o)+cur->GetSeriesOffset()));
        parm = (uint8_t**)((o)+cur->GetSeriesOffset());
        uint8_t** ppstop = (uint8_t**)((uint8_t*)parm + cur->GetSeriesSize() + (size));
        while (parm < ppstop)
        {
            if (!exp(parm))
                return false;
            parm++;
        }
        cur--;

    } while (cur >= last);

    return true;
}

// Object manipulation apis
dotnet_error embeddingapi_impl_readmanagedmem(dotnet_frame frame, CorElementType et, TypeHandle th, uint8_t* pPointerToManagedMemory, uint8_t* pUnmanagedData, int32_t cbDataUnmanaged)
{
    UINT size = GetSizeForCorElementType(et);
    if (size == (UINT) -1)
        size = th.GetMethodTable()->GetNumInstanceFieldBytes();

    if (CorTypeInfo::IsObjRef(et))
        th = TypeHandle(g_pObjectClass);

    if (((int32_t)size) > cbDataUnmanaged)
        return E_INVALIDARG;
    
    memcpy(pUnmanagedData, pPointerToManagedMemory, size);

    if (th != NULL)
    {
        dotnet_error failresult = S_OK;

        enumerate_gc_references_in_field_layout(th, pUnmanagedData, [frame, &failresult](uint8_t** pointer)
            {
                if (failresult == S_OK)
                {
                    dotnet_object newHandle;
                    failresult = embeddingapi_handle_alloc(frame, *(OBJECTREF*)pointer, &newHandle);
                    *(dotnet_object*)pointer = newHandle;
                }
                return !!SUCCEEDED(failresult);
            });
        return failresult;
    }
    else
    {
        return S_OK;
    }
}

dotnet_error embeddingapi_impl_writemanagedmem(CorElementType et, TypeHandle th, uint8_t* pPointerToManagedMemory, uint8_t* pUnmanagedData, int32_t cbDataUnmanaged)
{
    CONTRACTL
    {
        THROWS;
        GC_NOTRIGGER;
        MODE_COOPERATIVE;
        EE_THREAD_REQUIRED;
    }
    CONTRACTL_END;

    UINT size = GetSizeForCorElementType(et);
    if (size == (UINT) -1)
        size = th.GetMethodTable()->GetNumInstanceFieldBytes();

    if (CorTypeInfo::IsObjRef(et))
        th = TypeHandle(g_pObjectClass);

    if (((int32_t)size) > cbDataUnmanaged)
        return E_INVALIDARG;
    
    UINT bytesCopiedSoFar = 0;

    if (th != NULL)
    {
        enumerate_gc_references_in_field_layout(th, pUnmanagedData, [&bytesCopiedSoFar, pUnmanagedData, pPointerToManagedMemory](uint8_t** pointer)
            {
                // Copy non-reference object data
                UINT offsetFromStart = (UINT)((uint8_t*)pointer - pUnmanagedData);
                UINT additionalBytesToCopy = offsetFromStart - bytesCopiedSoFar;
                if (additionalBytesToCopy > 0)
                {
                    memcpy(pPointerToManagedMemory + bytesCopiedSoFar, pUnmanagedData + bytesCopiedSoFar, additionalBytesToCopy);
                    bytesCopiedSoFar += additionalBytesToCopy;
                }
                OBJECTREF* objectRefInManagedHeap = (OBJECTREF*)(pPointerToManagedMemory + bytesCopiedSoFar);
                dotnet_object* objectInNonManagedMemory = (dotnet_object*)(pUnmanagedData + bytesCopiedSoFar);
                SetObjectReference(objectRefInManagedHeap, embeddingapi_get_target(*objectInNonManagedMemory));
                bytesCopiedSoFar += sizeof(OBJECTREF);
                return true;
            });
    }

    // Copy any remaining non-reference objects
    {
        UINT additionalBytesToCopy = size - bytesCopiedSoFar;
        memcpy(pPointerToManagedMemory + bytesCopiedSoFar, pUnmanagedData + bytesCopiedSoFar, additionalBytesToCopy);
    }

    return S_OK;
}

dotnet_error embeddingapi_read_field(dotnet_frame frame, dotnet_object obj, dotnet_fieldid field, void*pData, int32_t cbData)
{
    FieldDesc *pFD = (FieldDesc*)field;

    if (field == NULL)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    CorElementType et = pFD->GetFieldType();

    TypeHandle th = NULL;
    if (et == ELEMENT_TYPE_VALUETYPE)
        th = pFD->GetApproxFieldTypeHandleThrowing();

    GCX_COOP();

    OBJECTREF objRef = embeddingapi_get_target(obj);

    result = embeddingapi_impl_readmanagedmem(frame, et, th, (uint8_t*)pFD->GetInstanceAddress(objRef), (uint8_t*)pData, cbData);
    END_EMBEDDING_API;
}

void embeddingapi_read_field_on_rawobject(dotnet_rawobject obj, dotnet_fieldid field, void* pData, int32_t cbData)
{
    // This api does no error checking, or anything, and are not protected by the ENTER_EMBEDDING_API macro
    FieldDesc *pFD = (FieldDesc*)field;
    Object *pObj = (Object*)obj;
    PTR_BYTE pbOnHeap = pObj->GetData() + pFD->GetOffset_NoLogging();
    memcpy(pData, pbOnHeap, cbData);
}

void embeddingapi_write_field_on_rawobject(dotnet_rawobject obj, dotnet_fieldid field, void* pData, int32_t cbData)
{
    // This api does no error checking, or anything, and are not protected by the ENTER_EMBEDDING_API macro
    FieldDesc *pFD = (FieldDesc*)field;
    Object *pObj = (Object*)obj;
    PTR_BYTE pbOnHeap = pObj->GetData() + pFD->GetOffset_NoLogging();
    memcpy(pbOnHeap, pData, cbData);
}

dotnet_error embeddingapi_write_field(dotnet_object obj, dotnet_fieldid field, void*pData, int32_t cbData)
{
    FieldDesc *pFD = (FieldDesc*)field;

    if (field == NULL)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    CorElementType et = pFD->GetFieldType();

    TypeHandle th = NULL;
    if (et == ELEMENT_TYPE_VALUETYPE)
        th = pFD->GetApproxFieldTypeHandleThrowing();

    GCX_COOP();

    OBJECTREF objRef = embeddingapi_get_target(obj);

    result = embeddingapi_impl_writemanagedmem(et, th, (uint8_t*)pFD->GetInstanceAddress(objRef), (uint8_t*)pData, cbData);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_read_static_field(dotnet_frame frame, dotnet_fieldid field, void*pData, int32_t cbData)
{
    FieldDesc *pFD = (FieldDesc*)field;

    if (field == NULL)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    CorElementType et = pFD->GetFieldType();

    TypeHandle th = NULL;
    if (et == ELEMENT_TYPE_VALUETYPE)
        th = pFD->GetApproxFieldTypeHandleThrowing();

    GCX_COOP();

    pFD->GetApproxEnclosingMethodTable_NoLogging()->CheckRunClassInitThrowing();

    result = embeddingapi_impl_readmanagedmem(frame, et, th, (uint8_t*)pFD->GetCurrentStaticAddress(), (uint8_t*)pData, cbData);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_write_static_field(dotnet_fieldid field, void*pData, int32_t cbData)
{
    FieldDesc *pFD = (FieldDesc*)field;

    if (field == NULL)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    CorElementType et = pFD->GetFieldType();

    TypeHandle th = NULL;
    if (et == ELEMENT_TYPE_VALUETYPE)
        th = pFD->GetApproxFieldTypeHandleThrowing();

    GCX_COOP();

    pFD->GetApproxEnclosingMethodTable_NoLogging()->CheckRunClassInitThrowing();

    result = embeddingapi_impl_writemanagedmem(et, th, (uint8_t*)pFD->GetCurrentStaticAddress(), (uint8_t*)pData, cbData);
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_read_field_on_struct(void *structure, dotnet_fieldid field, void *data, int32_t cbData)
{
    FieldDesc *pFD = (FieldDesc*)field;

    if (field == NULL)
        return E_INVALIDARG;
    
    ENTER_EMBEDDING_API;

    DWORD offset = pFD->GetOffset();
    UINT size = pFD->GetSize();

    if (cbData < (int32_t)size)
        result = E_INVALIDARG;
    else
        memcpy(data, ((uint8_t*)structure) + offset, size);
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_write_field_on_struct(void *structure, dotnet_fieldid field, void *data, int32_t cbData)
{
    FieldDesc *pFD = (FieldDesc*)field;

    if (field == NULL)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    DWORD offset = pFD->GetOffset();
    UINT size = pFD->GetSize();

    if (cbData < (int32_t)size)
        result = E_INVALIDARG;
    else
        memcpy(((uint8_t*)structure) + offset, data, size);
    END_EMBEDDING_API;
}

dotnet_error embeddingapi_array_read_element(dotnet_frame frame, dotnet_object array, int32_t index, void* pData, int32_t cbData)
{
    if (array == NULL)
        return E_INVALIDARG;

    if (index < 0)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    GCX_COOP();

    ARRAYBASEREF objRef = (ARRAYBASEREF)embeddingapi_get_target(array);
    CorElementType et = objRef->GetArrayElementType(); 
    TypeHandle th = NULL;
    if (et == ELEMENT_TYPE_VALUETYPE)
        th = objRef->GetArrayElementTypeHandle();

    result = embeddingapi_impl_readmanagedmem(frame, et, th, (uint8_t*)objRef->GetDataPtr() + (index * cbData), (uint8_t*)pData, cbData);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_array_write_element(dotnet_object array, int32_t index, void* pData, int32_t cbData)
{
    if (array == NULL)
        return E_INVALIDARG;

    if (index < 0)
        return E_INVALIDARG;

    ENTER_EMBEDDING_API;

    GCX_COOP();

    ARRAYBASEREF objRef = (ARRAYBASEREF)embeddingapi_get_target(array);
    CorElementType et = objRef->GetArrayElementType(); 
    TypeHandle th = NULL;
    if (et == ELEMENT_TYPE_VALUETYPE)
        th = objRef->GetArrayElementTypeHandle();

    result = embeddingapi_impl_writemanagedmem(et, th, (uint8_t*)objRef->GetDataPtr() + (index * cbData), (uint8_t*)pData, cbData);

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_register_eager_finalization_callback(dotnet_typeid type, dotnet_eagerfinalizeobjectcallback finalizeCallback)
{
    ENTER_EMBEDDING_API;

    TypeHandle th = TypeHandle::FromPtr((void*)type);
    if (th.IsTypeDesc())
        result = E_INVALIDARG;
    else
    {
        MethodTable* pMT = th.AsMethodTable();

        if (!pMT->IsEagerFinalized())
        {
            result = E_INVALIDARG;
        }
        else
        {
            SetEagerFinalizer(pMT, (EagerFinalizer)finalizeCallback);
        }
    }

    END_EMBEDDING_API;
}

dotnet_error embeddingapi_alloc_callbacks(void* callbacksptr, dotnet_runtime_callbacks_handle *callbacks_handle)
{
    if (embedding_api_handle_allocated)
        return E_OUTOFMEMORY; // We don't actually support multiplexing at this time
    
    embedding_api_handle_allocated = true;
    embedding_api_callbacksptr = callbacksptr;
    *callbacks_handle = (dotnet_runtime_callbacks_handle)1;
    return S_OK;
}

dotnet_error embeddingapi_set_thread_started_callback(dotnet_runtime_callbacks_handle callbackshandle, dotnet_threadstarted threadstarted)
{
    if (callbackshandle != (dotnet_runtime_callbacks_handle)1)
        return E_INVALIDARG;
    
    embedding_api_thread_started = threadstarted;
    return S_OK;
}

dotnet_error embeddingapi_set_thread_stopped_callback(dotnet_runtime_callbacks_handle callbackshandle, dotnet_threadstopped threadstopped)
{
    if (callbackshandle != (dotnet_runtime_callbacks_handle)1)
        return E_INVALIDARG;
    
    embedding_api_thread_stopped = threadstopped;
    return S_OK;
}

dotnet_error embeddingapi_set_gc_event_callback(dotnet_runtime_callbacks_handle callbackshandle, dotnet_gc_event gcevent)
{
    if (callbackshandle != (dotnet_runtime_callbacks_handle)1)
        return E_INVALIDARG;
    
    embedding_api_gc_event = gcevent;
    return S_OK;
}

// Setup embedding api infrastructure
static void* GetApiFromManaged(GetApiHelperEnum api)
{
    MethodDescCallSite getApi(METHOD__EMBEDDING_API__GET_API);
    ARG_SLOT args[1] = { (ARG_SLOT)api };
    return getApi.Call_RetLPVOID(args);
}

dotnet_error embeddingapi_getapi(const char *apiname, void** functions, int functionsBufferSizeInBytes)
{
    ENTER_EMBEDDING_API;

    if (apiname == NULL)
        result = E_INVALIDARG;
    else
    {
        GCX_COOP();

        if (strcmp(apiname, DOTNET_V1_API_GROUP) == 0)
        {
            if (sizeof(dotnet_embedding_api_group) > functionsBufferSizeInBytes)
            {
                result = E_INVALIDARG;
            }
            else
            {
                dotnet_embedding_api_group *pApi = (dotnet_embedding_api_group *)functions;
                memset(pApi, 0, sizeof(dotnet_embedding_api_group));

#define SET_API(api) pApi->api = embeddingapi_ ## api
#define MANAGED_API(api) pApi->api = (decltype(pApi->api))GetApiFromManaged(GetApiHelperEnum::api)

                SET_API(alloc);
                SET_API(free);

                SET_API(push_frame);
                SET_API(push_frame_collect_on_return);
                SET_API(pop_frame);
                SET_API(handle_free);
                SET_API(handle_pin);
                SET_API(handle_unpin);

                MANAGED_API(object_tostring);
                SET_API(object_gettypeid);
                SET_API(object_isinst);
//                SET_API(object_resolve_virtual);
                SET_API(object_alloc);

                SET_API(gchandle_alloc);
                SET_API(gchandle_alloc_weak);
                SET_API(gchandle_alloc_weak_track_resurrection);
                SET_API(gchandle_free);
                SET_API(gchandle_get_target);

                MANAGED_API(type_gettype);
                MANAGED_API(type_getmethod);
                MANAGED_API(type_getfield);
                MANAGED_API(type_get_element_type);
                MANAGED_API(type_getconstructor);

                SET_API(get_typeid);
                SET_API(get_methodid);
                SET_API(get_fieldid);
//                SET_API(get_field_typeid);
                SET_API(get_type_from_typeid);
                MANAGED_API(get_method_from_methodid);
                SET_API(typeid_is_assignable_from);
                SET_API(typeid_is_valuetype);
                SET_API(typeid_field_size);
                SET_API(typeid_is_enum);
                SET_API(typeid_is_class);
                SET_API(typeid_enum_underlying_type);
                SET_API(typeid_is_byref);
                SET_API(methodid_get_signature);
                SET_API(methodsignature_free);
                SET_API(methodsignature_get_argument_count);
                SET_API(methodsignature_is_instance);
                SET_API(methodsignature_get_return_typeid);
                SET_API(methodsignature_get_nextarg_typeid);
//                SET_API(get_method_typeid);

                SET_API(read_field);
                SET_API(write_field);
                SET_API(read_static_field);
                SET_API(write_static_field);
                SET_API(read_field_on_struct);
                SET_API(write_field_on_struct);

                SET_API(box);
                SET_API(unbox);

                MANAGED_API(string_alloc_utf8);
                MANAGED_API(utf8_getstring);

                SET_API(method_invoke);

                SET_API(array_read_element);
                SET_API(array_write_element);

                SET_API(toggleref_creategroup);
//                SET_API(toggleref_destroygroup);
                SET_API(toggleref_alloc);
                SET_API(toggleref_free);
                SET_API(toggleref_get_target);

                SET_API(register_eager_finalization_callback);

                SET_API(alloc_callbacks);
                SET_API(set_thread_started_callback);
                SET_API(set_thread_stopped_callback);
                SET_API(set_gc_event_callback);

                SET_API(read_field_on_rawobject);
                SET_API(write_field_on_rawobject);
                result = S_OK;
            }
        }
        else
        {
            result = E_INVALIDARG;
        }
    }
    END_EMBEDDING_API;
}
