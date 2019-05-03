// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include <windows.h>
#include <dotnet_embedding.h>
#include <stdio.h>

#define THROW_IF_FAILED(exp) { hr = exp; if (FAILED(hr)) { ::printf("FAILURE: 0x%08x = %s\n", hr, #exp); throw hr; } }
#define THROW_FAIL_IF_FALSE(exp) { if (!(exp)) { ::printf("FALSE: %s\n", #exp); throw E_FAIL; } }

dotnet_embedding_api_group dotnet;

void CallWriteLineHelloWorld();

int __cdecl main()
{
    HMODULE hmodCoreShim = LoadLibraryW(L"coreshim.dll");
    if (hmodCoreShim == NULL)
    {
        ::printf("Loading coreshim failed\n");
        return 101;
    }
    dotnet_getapi getapi = (dotnet_getapi)GetProcAddress(hmodCoreShim, "coreclr_getapi");
    if (getapi == NULL)
    {
        ::printf("Getting getapi entrypoint failed\n");
        return 101;
    }

    dotnet_error hr;

    if (FAILED(hr = getapi(DOTNET_V1_API_GROUP, (void**)&dotnet, sizeof(dotnet))))
    {
        ::printf("Unable to call getapi %d\n", hr);
        return 101;
    }

    try
    {
        CallWriteLineHelloWorld();
    }
    catch (HRESULT hr)
    {
        ::printf("Test Failure: 0x%08x\n", hr);
        return 101;
    }

    return 100;
}

void CallWriteLineHelloWorld()
{
    ::printf("Printing helloworld via embedding api...\n");
}
