// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __DOTNET_EMBEDDING_H__
#define __DOTNET_EMBEDDING_H__

#include <stdint.h>

typedef struct { void* _unused; } _struct_dotnet_object;
typedef _struct_dotnet_object* dotnet_object;
typedef struct { void* _unused; } _struct_dotnet_rawobject;
typedef _struct_dotnet_rawobject* dotnet_rawobject;
typedef struct { void* _unused; } _struct_dotnet_pin;
typedef _struct_dotnet_pin* dotnet_pin;
typedef struct { void* _unused; } _struct_dotnet_gchandle;
typedef _struct_dotnet_gchandle* dotnet_gchandle;
typedef struct { void* _unused; } _struct_dotnet_frame;
typedef _struct_dotnet_frame* dotnet_frame;
typedef struct { void* _unused; } _struct_dotnet_typeid;
typedef _struct_dotnet_typeid* dotnet_typeid;
typedef struct { void* _unused; } _struct_dotnet_methodid;
typedef _struct_dotnet_methodid* dotnet_methodid;
typedef struct { void* _unused; } _struct_dotnet_fieldid;
typedef _struct_dotnet_fieldid* dotnet_fieldid;

typedef struct { void* _unused; } _struct_dotnet_toggleref;
typedef _struct_dotnet_toggleref* dotnet_toggleref;
typedef struct { void* _unused; } _struct_dotnet_togglerefgroup;
typedef _struct_dotnet_togglerefgroup* dotnet_togglerefgroup;
typedef struct { void* _unused; } _struct_dotnet_methodsignature;
typedef _struct_dotnet_methodsignature* dotnet_methodsignature;

typedef struct { void* _unused; } _dotnet_runtime_callbacks_handle;
typedef _dotnet_runtime_callbacks_handle* dotnet_runtime_callbacks_handle;

typedef uint32_t dotnet_error;

typedef int32_t (*dotnet_togglerefcallback)(dotnet_rawobject object);

typedef int32_t (*dotnet_eagerfinalizeobjectcallback)(dotnet_rawobject object);

typedef void (*dotnet_threadstarted)(void* callbacksptr, uintptr_t tid);
typedef void (*dotnet_threadstopped)(void* callbacksptr, uintptr_t tid);

typedef enum
{
    dotnet_gc_event_pre_stop_world = 6,
    dotnet_gc_event_post_start_world = 9,
} dotnet_gc_events;

typedef void (*dotnet_gc_event)(void *callbacksptr, dotnet_gc_events gcevent, int generation, int32_t isSerial);

typedef enum
{
    dotnet_bindingflags_Default = 0,
    dotnet_bindingflags_IgnoreCase = 1,
    dotnet_bindingflags_DeclaredOnly = 2,
    dotnet_bindingflags_Instance = 4,
    dotnet_bindingflags_Static = 8,
    dotnet_bindingflags_Public = 16,
    dotnet_bindingflags_NonPublic = 32,
    dotnet_bindingflags_FlattenHierarchy = 64,
    dotnet_bindingflags_InvokeMethod = 256,
    dotnet_bindingflags_CreateInstance = 512,
    dotnet_bindingflags_GetField = 1024,
    dotnet_bindingflags_SetField = 2048,
    dotnet_bindingflags_GetProperty = 4096,
    dotnet_bindingflags_SetProperty = 8192,
    dotnet_bindingflags_PutDispProperty = 16384,
    dotnet_bindingflags_PutDispRefProperty = 32768,
    dotnet_bindingflags_ExactBinding = 65536,
    dotnet_bindingflags_SuppressChangeType = 131072,
    dotnet_bindingflags_OptionalParamBinding = 262144,
    dotnet_bindingflags_IgnoreReturn = 16777216,
    dotnet_bindingflags_DoNotWrapExceptions = 33554432,
} dotnet_bindingflags;

typedef struct
{
    dotnet_typeid type;
    void* data;
    int32_t cbdata;
} dotnet_invokeargument;

typedef enum
{
    dotnet_methodinvoke_exception_catch = 0,
    dotnet_methodinvoke_exception_flowthrough = 1,
} dotnet_methodinvoke_flags;

typedef dotnet_error (*dotnet_getapi)(const char* apiname, void** pFunctions, int bufferSizeInBytes);

typedef dotnet_error(*_dotnet_uint32_out_voidptr)(uint32_t,void**);
typedef dotnet_error(*_dotnet_voidptr)(void*);
typedef dotnet_error(*_dotnet_object_out_gchandle)(dotnet_object, dotnet_gchandle*);
typedef dotnet_error(*_dotnet_gchandle)(dotnet_gchandle);
typedef dotnet_error(*_dotnet_gchandle_out_int8)(dotnet_gchandle, int8_t*);
typedef dotnet_error(*_dotnet_frame_gchandle_out_object)(dotnet_frame, dotnet_gchandle, dotnet_object*);
typedef dotnet_error(*_dotnet_gchandle_object)(dotnet_gchandle, dotnet_object);
typedef dotnet_error(*_dotnet_gchandle_gchandle_out_int8)(dotnet_gchandle,dotnet_gchandle, int8_t*);
typedef dotnet_error(*_dotnet_gchandle_object_out_int8)(dotnet_gchandle,dotnet_object, int8_t*);
typedef dotnet_error(*_out_dotnet_frame)(dotnet_frame *);
typedef dotnet_error(*_dotnet_frame)(dotnet_frame);
typedef dotnet_error(*_dotnet_frame_object)(dotnet_frame,dotnet_object);
typedef dotnet_error(*_dotnet_object_object_out_int8)(dotnet_object,dotnet_object,int8_t*);
typedef dotnet_error(*_dotnet_frame_object_out_object)(dotnet_frame,dotnet_object,dotnet_object*);
typedef dotnet_error(*_dotnet_object_out_typeid)(dotnet_object,dotnet_typeid*);
typedef dotnet_error(*_dotnet_frame_object_out_pin_out_voidptr)(dotnet_frame,dotnet_object,dotnet_pin*,void**);
typedef dotnet_error(*_dotnet_frame_pin)(dotnet_frame,dotnet_pin);
typedef dotnet_error(*_dotnet_frame_utf8str_out_object)(dotnet_frame,const char *,dotnet_object*);
typedef dotnet_error(*_dotnet_frame_object_utf8str_bindingflags_objectptr_int32_out_method)(dotnet_frame, dotnet_object, const char*, dotnet_bindingflags, dotnet_object*, int32_t, dotnet_object*);
typedef dotnet_error(*_dotnet_frame_object_utf8str_out_object)(dotnet_frame, dotnet_object, const char*, dotnet_object*);
typedef dotnet_error(*_dotnet_object_out_typeid)(dotnet_object,dotnet_typeid*);
typedef dotnet_error(*_dotnet_object_out_methodid)(dotnet_object,dotnet_methodid*);
typedef dotnet_error(*_dotnet_object_out_fieldid)(dotnet_object,dotnet_fieldid*);
typedef dotnet_error(*_dotnet_fieldid_out_typeid)(dotnet_fieldid,dotnet_typeid*);
typedef dotnet_error(*_dotnet_frame_typeid_out_object)(dotnet_frame,dotnet_typeid,dotnet_object*);
typedef dotnet_error(*_dotnet_frame_methodid_out_object)(dotnet_frame,dotnet_methodid,dotnet_object*);
typedef dotnet_error(*_dotnet_frame_methodid_invokeargumentptr_int32_methodinvokeflags)(dotnet_frame,dotnet_methodid,dotnet_invokeargument*,int32_t,dotnet_methodinvoke_flags);
typedef dotnet_error(*_dotnet_frame_object_fieldid_voidptr_int32)(dotnet_frame,dotnet_object,dotnet_fieldid,void*,int32_t);
typedef dotnet_error(*_dotnet_object_fieldid_voidptr_int32)(dotnet_object,dotnet_fieldid,void*,int32_t);
typedef dotnet_error(*_dotnet_frame_fieldid_voidptr_int32)(dotnet_frame,dotnet_fieldid,void*,int32_t);
typedef dotnet_error(*_dotnet_fieldid_voidptr_int32)(dotnet_fieldid,void*,int32_t);
typedef dotnet_error(*_dotnet_voidptr_fieldid_voidptr_int32)(void*,dotnet_fieldid,void*,int32_t);
typedef dotnet_error(*_dotnet_togglerefcallback_out_togglerefgroup)(dotnet_togglerefcallback, dotnet_togglerefgroup*);
typedef dotnet_error(*_dotnet_togglerefgroup)(dotnet_togglerefgroup);
typedef dotnet_error(*_dotnet_togglerefgroup_object_out_toggleref)(dotnet_togglerefgroup, dotnet_object, dotnet_toggleref*);
typedef dotnet_error(*_dotnet_togglerefgroup_toggleref)(dotnet_togglerefgroup, dotnet_toggleref);
typedef dotnet_error(*_dotnet_frame_togglerefgroup_toggleref_out_object)(dotnet_frame, dotnet_togglerefgroup, dotnet_toggleref, dotnet_object*);
typedef dotnet_error(*_dotnet_typeid_eagerfinalizeobjectcallback)(dotnet_typeid, dotnet_eagerfinalizeobjectcallback);
typedef dotnet_error(*_dotnet_voidptr_out_runtime_callbacks_handle)(void*, dotnet_runtime_callbacks_handle*);
typedef dotnet_error(*_dotnet_runtime_callbacks_handle_threadstarted)(dotnet_runtime_callbacks_handle, dotnet_threadstarted);
typedef dotnet_error(*_dotnet_runtime_callbacks_handle_threadstopped)(dotnet_runtime_callbacks_handle, dotnet_threadstopped);
typedef dotnet_error(*_dotnet_runtime_callbacks_handle_gc_event)(dotnet_runtime_callbacks_handle, dotnet_gc_event);
typedef dotnet_error(*_dotnet_typeid_typeid_out_int32)(dotnet_typeid, dotnet_typeid, int32_t*);
typedef dotnet_error(*_dotnet_typeid_out_int32)(dotnet_typeid, int32_t*);
typedef dotnet_error(*_dotnet_object_out_utf8str_out_int32)(dotnet_object, char**, int32_t*);
typedef dotnet_error(*_dotnet_object_out_int32)(dotnet_object, int32_t*);
typedef dotnet_error(*_dotnet_object_typeid_int32)(dotnet_object, dotnet_typeid, int32_t*);
typedef dotnet_error(*_dotnet_object_methodid_out_methodid)(dotnet_object, dotnet_methodid, dotnet_methodid*);
typedef dotnet_error(*_dotnet_frame_typeid_voidptr_int32_out_object)(dotnet_frame, dotnet_typeid, void*, int32_t, dotnet_object*);
typedef dotnet_error(*_dotnet_object_voidptr_int32)(dotnet_object, void*, int32_t);
typedef dotnet_error(*_dotnet_frame_object_bindingflags_objectptr_int32_out_object)(dotnet_frame, dotnet_object, dotnet_bindingflags, dotnet_object*, int32_t, dotnet_object*);
typedef dotnet_error(*_dotnet_frame_object_int32_voidptr_int32)(dotnet_frame, dotnet_object, int32_t, void*, int32_t);
typedef dotnet_error(*_dotnet_object_int32_voidptr_int32)(dotnet_object, int32_t, void*, int32_t);
typedef void(*_dotnet_rawobject_fieldid_voidptr_int32)(dotnet_rawobject,dotnet_fieldid,void*,int32_t);
typedef dotnet_error(*_dotnet_typeid_out_typeid)(dotnet_typeid, dotnet_typeid*);
typedef dotnet_error(*_dotnet_methodid_out_methodsignature)(dotnet_methodid, dotnet_methodsignature*);
typedef dotnet_error(*_dotnet_methodsignature)(dotnet_methodsignature);
typedef dotnet_error(*_dotnet_methodsignature_out_int32)(dotnet_methodsignature, int32_t*);
typedef dotnet_error(*_dotnet_methodsignature_out_typeid)(dotnet_methodsignature, dotnet_typeid*);
typedef dotnet_error(*_dotnet_methodid_out_typeid)(dotnet_methodid, dotnet_typeid*);
typedef dotnet_error(*_dotnet_frame_object_utf8str_bindingflags_out_object)(dotnet_frame, dotnet_object, const char *, dotnet_bindingflags, dotnet_object*);
typedef dotnet_error(*_dotnet_frame_methodid_typeid_out_object)(dotnet_frame,dotnet_methodid,dotnet_typeid,dotnet_object*);
typedef dotnet_error(*_dotnet_frame_object_typeid_voidptr_int32)(dotnet_frame, dotnet_object, dotnet_typeid, void*,int32_t);

#define DOTNET_V1_API_GROUP "DOTNET.0"
typedef struct _dotnet_embedding_api_group
{
    // Utility
    _dotnet_uint32_out_voidptr alloc;
    _dotnet_voidptr free;
    
    // Thread local handle functions
    _out_dotnet_frame push_frame;
    _out_dotnet_frame push_frame_collect_on_return;
    _dotnet_frame pop_frame;
    _dotnet_frame_object handle_free;
    _dotnet_frame_object_out_pin_out_voidptr handle_pin;
    _dotnet_frame_pin handle_unpin;

    // Object functionality
//    _dotnet_object_object_out_int8 object_reference_equal;
    _dotnet_frame_object_out_object object_tostring;
//    _dotnet_frame_object_out_object object_gettype;
    _dotnet_object_out_typeid object_gettypeid;
    _dotnet_object_typeid_int32 object_isinst;
//1    _dotnet_object_methodid_out_methodid object_resolve_virtual;
    _dotnet_frame_typeid_out_object object_alloc;
 
    // GCHandle functions
    _dotnet_object_out_gchandle gchandle_alloc;
    _dotnet_object_out_gchandle gchandle_alloc_weak;
    _dotnet_object_out_gchandle gchandle_alloc_weak_track_resurrection;
    _dotnet_gchandle gchandle_free;
    _dotnet_frame_gchandle_out_object gchandle_get_target;
//    _dotnet_gchandle_object gchandle_set_target;
//    _dotnet_gchandle_gchandle_out_int8 gchandle_reference_equal;
//    _dotnet_gchandle_object_out_int8 gchandle_reference_equal_object;
//    _dotnet_gchandle_out_int8 gchandle_isnull;

    // Reflection surface
    _dotnet_frame_utf8str_out_object type_gettype;
    _dotnet_frame_object_utf8str_bindingflags_objectptr_int32_out_method type_getmethod;
    _dotnet_frame_object_utf8str_bindingflags_out_object type_getfield; // ADDED
    _dotnet_frame_object_out_object type_get_element_type;
    _dotnet_frame_object_bindingflags_objectptr_int32_out_object type_getconstructor;

    // Non-managed type system structure access
    _dotnet_object_out_typeid get_typeid;
    _dotnet_object_out_methodid get_methodid;
    _dotnet_object_out_fieldid get_fieldid;
//1    _dotnet_fieldid_out_typeid get_field_typeid;
//    _dotnet_frame_typeid_out_object get_typeid_lifetime_object;
//    _dotnet_frame_methodid_out_object get_methodid_lifetime_object;
    _dotnet_frame_typeid_out_object get_type_from_typeid; // ADDED
    _dotnet_frame_methodid_typeid_out_object get_method_from_methodid; // ADDED
    _dotnet_typeid_typeid_out_int32 typeid_is_assignable_from;
    _dotnet_typeid_out_int32 typeid_is_valuetype;
    _dotnet_typeid_out_int32 typeid_field_size;
    _dotnet_typeid_out_int32 typeid_is_enum;
    _dotnet_typeid_out_int32 typeid_is_class;
    _dotnet_typeid_out_typeid typeid_enum_underlying_type;
    _dotnet_typeid_out_int32 typeid_is_byref;
    _dotnet_methodid_out_methodsignature methodid_get_signature;
    _dotnet_methodsignature methodsignature_free;
    _dotnet_methodsignature_out_int32 methodsignature_get_argument_count;
    _dotnet_methodsignature_out_int32 methodsignature_is_instance;
    _dotnet_methodsignature_out_typeid methodsignature_get_return_typeid;
    _dotnet_methodsignature_out_typeid methodsignature_get_nextarg_typeid;
    _dotnet_methodid_out_typeid get_method_typeid;

    
    // Object manipulation
    _dotnet_frame_object_fieldid_voidptr_int32 read_field;
    _dotnet_object_fieldid_voidptr_int32 write_field;
    _dotnet_frame_fieldid_voidptr_int32 read_static_field;
    _dotnet_fieldid_voidptr_int32 write_static_field;
    _dotnet_voidptr_fieldid_voidptr_int32 read_field_on_struct;
    _dotnet_voidptr_fieldid_voidptr_int32 write_field_on_struct;
    _dotnet_frame_typeid_voidptr_int32_out_object box;
    _dotnet_frame_object_typeid_voidptr_int32 unbox;

    // String api
    _dotnet_frame_utf8str_out_object string_alloc_utf8;
    _dotnet_object_out_utf8str_out_int32 utf8_getstring; // ADDED

    // Method Invoke
    _dotnet_frame_methodid_invokeargumentptr_int32_methodinvokeflags method_invoke;

    // arrays api
    _dotnet_object_out_int32 array_get_length;
    _dotnet_frame_object_int32_voidptr_int32 array_read_element; // ADDED
    _dotnet_object_int32_voidptr_int32 array_write_element;

    // Toggle refs
    _dotnet_togglerefcallback_out_togglerefgroup toggleref_creategroup;
//1    _dotnet_togglerefgroup toggleref_destroygroup;
    _dotnet_togglerefgroup_object_out_toggleref toggleref_alloc;
    _dotnet_togglerefgroup_toggleref toggleref_free;
    _dotnet_frame_togglerefgroup_toggleref_out_object toggleref_get_target;

    // Eager finalization
    _dotnet_typeid_eagerfinalizeobjectcallback register_eager_finalization_callback;

    // profiler like api surface
    _dotnet_voidptr_out_runtime_callbacks_handle alloc_callbacks;
    _dotnet_runtime_callbacks_handle_threadstarted set_thread_started_callback;
    _dotnet_runtime_callbacks_handle_threadstopped set_thread_stopped_callback;
    _dotnet_runtime_callbacks_handle_gc_event set_gc_event_callback;

    // Raw object access apis
    _dotnet_rawobject_fieldid_voidptr_int32 read_field_on_rawobject;
    _dotnet_rawobject_fieldid_voidptr_int32 write_field_on_rawobject;
} dotnet_embedding_api_group;

#endif // __DOTNET_EMBEDDING_H__
