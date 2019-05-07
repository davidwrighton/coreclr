// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// File: embeddingapi.cpp
//

#include "common.h"
#include <embeddingapi/dotnet_embedding.h>

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

ExpandoEmbeddingApiFrame::ExpandoEmbeddingApiFrame() : _expando(NULL), _remainingInitialFreeElements(s_elementCount)
{
}

/*static*/ bool ExpandoEmbeddingApiFrame::AllocEntry(ExpandoEmbeddingApiFrame** ppFrame, void *newValue, void **allocatedEntry)
{
    ExpandoEmbeddingApiFrame* pFrame = *ppFrame;
    if (pFrame->_remainingFreeElements > 0)
    {
        pFrame->_remainingFreeElements--;
        pFrame->_elements[_remainingFreeElements] = newValue;
        return true;
    }
    else
    {
        ExpandoEmbeddingApiFrame* pNewFrame = new(nothrow) ExpandoEmbeddingApiFrame(pFrame);
        if (pNewFrame == NULL)
            return false;

        pNewFrame->_remainingFreeElements = s_elementCount - 1;
        pNewFrame->_elements[s_elementCount - 1] = newValue;
        pFrame->_expando = pNewFrame;
        *ppFrame = pNewFrame;
    }
}

void ExpandoEmbeddingApiFrame::GCPromote(promote_func* fn, ScanContext* sc)
{
    for (int i = 15; i >= _remainingFreeElements; i--)
    {
        UINT_PTR ptrInFrame = (UINT_PTR)_elements[i];
        switch (ptrInFrame & 0x3)
        {
        case 0:
            fn((Object**)&_elements[i], sc, 0);
            break;
        case 1:
            fn((Object**)&_elements[i], sc, GC_CALL_INTERIOR | GC_CALL_PINNED);
            break;

        default:
            // Do nothing, these are in the freelist
        }
    }
}

ExpandoEmbeddingApiFrame::~ExpandoEmbeddingApiFrame()
{
    delete _expando;
}

FrameForEmbeddingApi::FrameForEmbeddingApi(Thread *thread) :_expando(&_initialFrame), _nextFreeElement(NULL), _thread(thread), _next(thread->EmbeddingApiFrame)
{
    thread->EmbeddingApiFrame = this;
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
    _ASSERTE(((UINT_PTR)entryToFree) & 0x3 == 0);
    FreeElement *newFreeElement = (FreeElement*)entryToFree;
    newFreeElement->_nextFreeElement = ((UINT_PTR)_nextFreeElement) + 2;
    _nextFreeElement = newFreeElement;
}

bool FrameForEmbeddingApi::Pop()
{
    if (_thread->EmbeddingApiFrame == this))
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
    _thread->EmbeddingApiFrame = this->_next;
}

dotnet_error embeddingapi_push_frame(dotnet_frame *frame)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_PREEMPTIVE;
        EE_THREAD_NOT_REQUIRED;
    }
    CONTRACTL_END;

    BEGIN_GETTHREAD_ALLOWED;
    Thread *pThread = GetThreadNULLOk();

    if (pThread == NULL)
        pThread = SetupThread();

    GCX_COOP();
    *frame = (dotnet_frame)new(nothrow) FrameForEmbeddingApi(pThread);

    dotnet_error result = (*frame) == NULL ? E_OUTOFMEMORY : S_OK;

    END_GETTHREAD_ALLOWED;

    return result;
}

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

    GCX_COOP();
    FrameForEmbeddingApi* actualFrame = (FrameForEmbeddingApi*)frame;
    actualFrame->FreeEntry(object);
    return S_OK;
}

HRESULT embeddingapi_handle_alloc(dotnet_frame, OBJECTREF objRef, dotnet_object *handle)
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
        return E_INVALIDARG;
    
    if (actualFrame->AllocEntry(objRef->GetAddress(), handle))
        return S_OK;
    else
        return E_OUTOFMEMORY;
}

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
        *data = objRef->GetAddress() + ArrayBase::GetDataPtrOffset(pMT)
    }
    else
    {
        *data = objRef->GetData();
    }

    if (actualFrame->AllocEntry(objRef->GetAddress() + 1, pin)) // The +1 provides information to the GC walker in the frame to indicate this is a pinned reference.
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

dotnet_error embeddingapi_getapi(const char *apiname, void** functions, int functionsBufferSizeInBytes)
{
    if (apiname == NULL)
        return E_INVALIDARG;

    if (strcmp(apiname, DOTNET_V1_API_GROUP)
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
        pApi->handle_free = embeddingapi_handle_pin;
        pApi->handle_pin = embeddingapi_handle_pin;
        pApi->handle_unpin = embeddingapi_handle_unpin;

    }

    return E_FAIL;
}
