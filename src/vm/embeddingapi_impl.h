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
    const static int s_elementCount = 16;
    ExpandoEmbeddingApiFrame *_expando;
    UINT32 _remainingFreeElements;
    void *_elements[s_elementCount];
public:
    friend class FrameForEmbeddingApi;
    ExpandoEmbeddingApiFrame();
    static bool AllocEntry(ExpandoEmbeddingApiFrame** ppFrame, void *newValue, void **allocatedEntry);
    void GCPromote(promote_func* fn, ScanContext* sc);
    ~ExpandoEmbeddingApiFrame();
};

class FrameForEmbeddingApi
{
    FrameForEmbeddingApi *_next;
    ExpandoEmbeddingApiFrame *_expando; // Next frame to allocate into
    Thread *_thread;
    FreeElement* _nextFreeElement;
    ExpandoEmbeddingApiFrame _initialFrame;

public:
    FrameForEmbeddingApi(Thread *thread);
    void GCPromote(promote_func* fn, ScanContext* sc);
    bool AllocEntry(void *newValue, void **allocatedEntry);
    void FreeEntry(void *entryToFree);
    bool Pop();
    ~FrameForEmbeddingApi();
};

#include "fcall.h"


class EmbeddingApi
{
public:
    enum GetApiHelperEnum
    {
        // Keep this in sync with the num in embeddingapi_impl.h
        Type_GetType,
        Type_GetMethod,
        String_AllocUtf8
    };

    static FCDECL0(dotnet_frame, nPushFrame);
    static FCDECL1(void, nPopFrame, dotnet_frame frame);
    static FCDECL1(Object*, nGetTarget, dotnet_object obj);
    static FCDECL2(dotnet_object, nAllocHandle, dotnet_frame frame, Object* objectUNSAFE);
};

dotnet_error embeddingapi_handle_alloc(dotnet_frame, OBJECTREF objRef, dotnet_object *handle);


#endif // _EMBEDDINGAPI_IMPL_H