// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// File: embeddingapi.cpp
//

#include "common.h"

// Utility functions
dotnet_error embeddingapi_alloc(uint32_t cb, void ** bytes)
{
    STANDARD_VM_CONTRACT;
    *bytes = (void*)new (nothrow)BYTE[cb];
    if (*bytes == NULL)
        return E_OUTOFMEMORY;
    else
        return S_OK;
}

dotnet_error embeddingapi_free(void * bytes)
{
    STANDARD_VM_CONTRACT;
    delete[] bytes;
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
    CONTRACTL
    {
        NOTHROW;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
        EE_THREAD_NOT_REQUIRED;
    }
    CONTRACTL_END;

    dotnet_error result = 0;

    BEGIN_GETTHREAD_ALLOWED;
    Thread *pThread = GetThreadNULLOk();

    if (pThread == NULL)
        pThread = SetupThread();

    GCX_COOP();
    *frame = (dotnet_frame)new(nothrow) FrameForEmbeddingApi(pThread);

    result = (*frame) == NULL ? E_OUTOFMEMORY : S_OK;

    END_GETTHREAD_ALLOWED;

    return result;
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
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_PREEMPTIVE;
        EE_THREAD_REQUIRED;
    }
    CONTRACTL_END;

    GCX_COOP();
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    return actualFrame->Pop() ? S_OK : E_INVALIDARG;
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
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_PREEMPTIVE;
        EE_THREAD_REQUIRED;
    }
    CONTRACTL_END;

    if (object == NULL)
        return S_OK;

    GCX_COOP();
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    actualFrame->FreeEntry(object);
    return S_OK;
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

FCIMPL1(Object*, EmbeddingApi::nGetTarget, dotnet_object obj)
{
    if (obj == NULL)
        return NULL;

    return *(Object**)obj;
}
FCIMPLEND;

dotnet_error embeddingapi_handle_pin(dotnet_frame frame, dotnet_object object, dotnet_pin* pin, void**data)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_PREEMPTIVE;
        EE_THREAD_REQUIRED;
    }
    CONTRACTL_END;

    if (object == NULL)
        return E_INVALIDARG;

    GCX_COOP();
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;

    OBJECTREF objRef = ObjectToOBJECTREF(*(Object **)object);
    MethodTable *pMT = objRef->GetMethodTable();

    if (!pMT->IsPinnable())
    {
        return E_INVALIDARG;
    }

    if (pMT->HasComponentSize())
    {
        *data = objRef->GetAddress() + ArrayBase::GetDataPtrOffset(pMT);
    }
    else
    {
        *data = objRef->GetData();
    }

    if (actualFrame->AllocEntry(objRef->GetAddress() + 1, (void**)pin)) // The +1 provides information to the GC walker in the frame to indicate this is a pinned reference.
        return S_OK;
    else
        return E_OUTOFMEMORY;
}

dotnet_error embeddingapi_handle_unpin(dotnet_frame frame, dotnet_pin pin)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_PREEMPTIVE;
        EE_THREAD_REQUIRED;
    }
    CONTRACTL_END;

    GCX_COOP();
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    actualFrame->FreeEntry(pin);
    return S_OK;
}

// Id access surface
dotnet_error embeddingapi_get_methodid(dotnet_object method, dotnet_methodid* methodid)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_PREEMPTIVE;
        EE_THREAD_REQUIRED;
    }
    CONTRACTL_END;

    if (method == NULL)
        return E_INVALIDARG;
    
    GCX_COOP();
    REFLECTMETHODREF methodObj = (REFLECTMETHODREF)ObjectToOBJECTREF(*(Object**)method);
    MethodDesc *pMD = methodObj->GetMethod();
    *methodid = (dotnet_methodid)pMD;
    return S_OK;
}

dotnet_error embeddingapi_get_typeid(dotnet_object typeObject, dotnet_typeid* typeidVar)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_PREEMPTIVE;
        EE_THREAD_REQUIRED;
    }
    CONTRACTL_END;

    if (typeObject == NULL)
        return E_INVALIDARG;
    
    GCX_COOP();
    REFLECTCLASSBASEREF typeObj = (REFLECTCLASSBASEREF)ObjectToOBJECTREF(*(Object**)typeObject);
    TypeHandle th = typeObj->GetType();
    *typeidVar = (dotnet_typeid)th.AsPtr();
    return S_OK;
}

// Method invoke implementation
dotnet_error embeddingapi_method_invoke(dotnet_frame frame, dotnet_methodid methodId, dotnet_invokeargument* arguments, int32_t countOfArguments, dotnet_methodinvoke_flags flags)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
        EE_THREAD_NOT_REQUIRED;
    }
    CONTRACTL_END;

    GCX_COOP();

    ARG_SLOT *pArgSlots = (ARG_SLOT*)_alloca(sizeof(ARG_SLOT) * countOfArguments - 1);

    MethodDescCallSite methodToCall((MethodDesc*)methodId);

    for (int i = 0; i < (countOfArguments - 1); i++)
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
                return E_FAIL;
            }
        }
        else if (etCallingConvention == ELEMENT_TYPE_BYREF)
        {
            TypeHandle thByRefOf = argumentTypeHandle.GetTypeParam();
            etCallingConvention = thByRefOf.GetInternalCorElementType();

            if (CorTypeInfo::IsObjRef(etCallingConvention))
            {
                // Case 6.
                return E_FAIL;
            }
            if (!argumentTypeHandle.GetMethodTable()->ContainsPointers())
            {
                // Case 5 and 7
                pArgSlots[i] = (ARG_SLOT)invokeArg.data;
            }
            else
            {
                // Case 8
                return E_FAIL;
            }
        }
        else
        {
            // Case 1
            _ASSERTE(argumentTypeHandle.GetSize() <= sizeof(ARG_SLOT));
            memcpy(&pArgSlots[i], invokeArg.data, argumentTypeHandle.GetSize());
        }
    }

    ARG_SLOT returnValue = methodToCall.Call_RetArgSlot(pArgSlots);

    // There are a variety of return paths...
    // Handle two of them

    TypeHandle returnTypeHandle = TypeHandle::FromPtr((void*)arguments[0].type);
    CorElementType etReturnType = returnTypeHandle.GetInternalCorElementType();

    if (CorTypeInfo::IsObjRef(etReturnType))
    {
        return embeddingapi_handle_alloc(frame, ArgSlotToObj(returnValue), (dotnet_object*)arguments[0].data);
    }
    else if (etReturnType == ELEMENT_TYPE_VALUETYPE)
    {
        return E_FAIL;
    }
    else
    {
        // Copy argslot contents back into data
        memcpy(arguments[0].data, &returnValue, returnTypeHandle.GetSize());
        return S_OK;
    }
}


// Setup embedding api infrastructure
static void* GetApiFromManaged(EmbeddingApi::GetApiHelperEnum api)
{
    MethodDescCallSite getApi(METHOD__EMBEDDING_API__GET_API);
    ARG_SLOT args[1] = { (ARG_SLOT)api };
    return getApi.Call_RetLPVOID(args);
}

dotnet_error embeddingapi_getapi(const char *apiname, void** functions, int functionsBufferSizeInBytes)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
        EE_THREAD_NOT_REQUIRED;
    }
    CONTRACTL_END;

    if (apiname == NULL)
        return E_INVALIDARG;

    BEGIN_GETTHREAD_ALLOWED;
    Thread *pThread = GetThreadNULLOk();

    if (pThread == NULL)
        pThread = SetupThread();

    GCX_COOP();

    if (strcmp(apiname, DOTNET_V1_API_GROUP) == 0)
    {
        if (sizeof(dotnet_embedding_api_group) > functionsBufferSizeInBytes)
            return E_INVALIDARG;
        
        dotnet_embedding_api_group *pApi = (dotnet_embedding_api_group *)functions;
        memset(pApi, 0, sizeof(dotnet_embedding_api_group));

        pApi->alloc = embeddingapi_alloc;
        pApi->free = embeddingapi_free;
        pApi->push_frame = embeddingapi_push_frame;
        pApi->push_frame_collect_on_return = embeddingapi_push_frame_collect_on_return;
        pApi->pop_frame = embeddingapi_pop_frame;
        pApi->handle_free = embeddingapi_handle_free;
        pApi->handle_pin = embeddingapi_handle_pin;
        pApi->handle_unpin = embeddingapi_handle_unpin;

        pApi->type_gettype = (_dotnet_frame_utf8str_out_object)GetApiFromManaged(EmbeddingApi::Type_GetType);
        pApi->type_getmethod = (_dotnet_frame_object_utf8str_bindingflags_objectptr_int32_out_method)GetApiFromManaged(EmbeddingApi::Type_GetMethod);
        pApi->string_alloc_utf8 = (_dotnet_frame_utf8str_out_object)GetApiFromManaged(EmbeddingApi::String_AllocUtf8);

        pApi->get_typeid = embeddingapi_get_typeid;
        pApi->get_methodid = embeddingapi_get_methodid;

        pApi->method_invoke = embeddingapi_method_invoke;
    }

    END_GETTHREAD_ALLOWED;

    return S_OK;
}
