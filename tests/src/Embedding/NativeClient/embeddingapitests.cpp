// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef WINDOWS
#include <dlfcn.h>
#define FAILED(x) (x < 0)
#else
#include <windows.h>
#endif
#include <dotnet_embedding.h>
#include <stdio.h>

#define THROW_IF_FAILED(exp) { err = exp; if (FAILED(err)) { ::printf("FAILURE: 0x%08x = %s\n", err, #exp); throw err; } }
#define THROW_FAIL_IF_FALSE(exp) { if (!(exp)) { ::printf("FALSE: %s\n", #exp); throw E_FAIL; } }
#define IF_FAIL_GO(exp) { err = exp; if (FAILED(err)) { ::printf("FAILURE: 0x%08x = %s\n", err, #exp); goto fail; }}

dotnet_embedding_api_group dotnet;

void CallWriteLineHelloWorld();

int 
#ifdef WINDOWS
__cdecl 
#endif
main()
{
#ifdef WINDOWS
    HMODULE hmodCoreShim = LoadLibraryW(L"coreshim.dll");
    if (hmodCoreShim == NULL)
    {
        ::printf("Loading coreshim failed\n");
        return 101;
    }
    dotnet_getapi getapi = (dotnet_getapi)GetProcAddress(hmodCoreShim, "coreclr_getapi");
#else
    void* coreclrmod = dlopen("coreclr", RTLD_GLOBAL);
    dotnet_getapi getapi = (dotnet_getapi)dlsym(coreclrmod, "coreclr_getapi");
#endif
    if (getapi == NULL)
    {
        ::printf("Getting getapi entrypoint failed\n");
        return 101;
    }

    dotnet_error err;

    if (FAILED(err = getapi(DOTNET_V1_API_GROUP, (void**)&dotnet, sizeof(dotnet))))
    {
        ::printf("Unable to call getapi %d\n", err);
        return 101;
    }

    try
    {
        CallWriteLineHelloWorld();
    }
    catch (dotnet_error hr)
    {
        ::printf("Test Failure: 0x%08x\n", hr);
        return 101;
    }

    return 100;
}

void CallWriteLineHelloWorld()
{
    // This function is written in C style single-entry single exit to illustrate how to handle failure cases in C
    ::printf("Printing helloworld via embedding api...\n");

    dotnet_frame frame = NULL;
    dotnet_error err;
    
    IF_FAIL_GO(dotnet.push_frame(&frame));

    dotnet_object console_type;
    IF_FAIL_GO(dotnet.type_gettype(frame, "System.Console,System.Console", &console_type));

    dotnet_object object_type;
    dotnet_typeid object_typeid;
    IF_FAIL_GO(dotnet.type_gettype(frame, "System.Object,System.Runtime", &object_type));
    IF_FAIL_GO(dotnet.get_typeid(object_type, &object_typeid));

    dotnet_object void_type;
    dotnet_typeid void_typeid;
    IF_FAIL_GO(dotnet.type_gettype(frame, "System.Void,System.Runtime", &void_type));
    IF_FAIL_GO(dotnet.get_typeid(void_type, &void_typeid));

    dotnet_object argumentsToConsoleWriteline[1];
    argumentsToConsoleWriteline[0] = object_type;
    dotnet_object writeline;
    dotnet_methodid writeline_id;
    IF_FAIL_GO(dotnet.type_getmethod(frame, console_type, "WriteLine", (dotnet_bindingflags)(dotnet_bindingflags_Public | dotnet_bindingflags_Static), argumentsToConsoleWriteline, 1, &writeline));
    IF_FAIL_GO(dotnet.get_methodid(writeline, &writeline_id));

    // Create helloworld string
    dotnet_object helloworld;
    IF_FAIL_GO(dotnet.string_alloc_utf8(frame, "Hello world", &helloworld));

    dotnet_invokeargument invokearguments[2];
    invokearguments[0].data = NULL;
    invokearguments[0].cbdata = 0;
    invokearguments[0].type = void_typeid;
    invokearguments[1].data = &helloworld;
    invokearguments[1].cbdata = sizeof(helloworld);
    invokearguments[1].type = object_typeid;

    IF_FAIL_GO(dotnet.method_invoke(frame, writeline_id, invokearguments, 2, dotnet_methodinvoke_exception_catch));

    dotnet.pop_frame(frame);
    return;
fail:
    if (frame != NULL)
        dotnet.pop_frame(frame);
    throw err;
}
