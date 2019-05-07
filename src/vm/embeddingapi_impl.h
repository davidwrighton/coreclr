// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// File: embeddingapi_impl.h
//

#ifndef _EMBEDDINGAPI_IMPL_H
#define _EMBEDDINGAPI_IMPL_H
#include <embeddingapi/dotnet_embedding.h>

class Thread;
class ExpandoEmbeddingApiFrame;
struct FreeElement
{
    UINT_PTR _nextFreeElement;
};

class ExpandoEmbeddingApiFrame
{
    const int s_elementCount = 16;
    ExpandoEmbeddingApiFrame *_expando;
    UINT32 _remainingFreeElements;
    void *_elements[s_elementCount];
public:
    ExpandoEmbeddingApiFrame() : _expando(NULL), _remainingInitialFreeElements(s_elementCount);
    static bool AllocEntry(ExpandoEmbeddingApiFrame** ppFrame, void *newValue, void **allocatedEntry);
    void GCPromote(promote_func* fn, ScanContext* sc);
    ~ExpandoEmbeddingApiFrame();
}

class FrameForEmbeddingApi
{
    FrameForEmbeddingApi *_next;
    ExpandoEmbeddingApiFrame *_expando; // Next frame to allocate into
    Thread *_thread;
    FreeElement* _nextFreeElement;
    ExpandoEmbeddingApiFrame _initialFrame;

public:
    FrameForEmbeddingApi(Thread *thread) :_expando(&_initialFrame), _nextFreeElement(NULL), _thread(thread), _next(thread->EmbeddingApiFrame);
    void GCPromote(promote_func* fn, ScanContext* sc);
    bool AllocEntry(void *newValue, void **allocatedEntry);
    void FreeEntry(void *entryToFree);
    bool Pop();
    ~FrameForEmbeddingApi();
}

dotnet_error embeddingapi_handle_alloc(dotnet_frame, OBJECTREF objRef, dotnet_object *handle);


#endif // _EMBEDDINGAPI_IMPL_H