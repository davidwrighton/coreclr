// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __DOTNET_EMBEDDING_H__
#define __DOTNET_EMBEDDING_H__

#include <stdint.h>

struct _struct_dotnet_object {};
typedef _struct_dotnet_object* dotnet_object;
struct _struct_dotnet_raw_object {};
typedef _struct_dotnet_raw_object* dotnet_raw_object;
struct _struct_dotnet_pin {};
typedef _struct_dotnet_pin* dotnet_pin;
struct _struct_dotnet_gchandle {};
typedef _struct_dotnet_gchandle* dotnet_gchandle;
struct _struct_dotnet_frame {};
typedef _struct_dotnet_frame* dotnet_frame;
struct _struct_dotnet_typeid {};
typedef _struct_dotnet_typeid* dotnet_typeid;
struct _struct_dotnet_methodid {};
typedef _struct_dotnet_methodid* dotnet_methodid;
struct _struct_dotnet_fieldid {};
typedef _struct_dotnet_fieldid* dotnet_fieldid;

struct _struct_dotnet_toggleref {};
typedef _struct_dotnet_toggleref* dotnet_toggleref;
struct _struct_dotnet_togglerefgroup {};
typedef _struct_dotnet_togglerefgroup* dotnet_togglerefgroup;

typedef uint32_t dotnet_error;

typedef int32_t (*dotnet_togglerefcallback)(dotnet_rawobject object);

enum dotnet_bindingflags
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
};

struct dotnet_invokeargument
{
	dotnet_typeid type;
	void* data;
	int32_t cbdata;
};
enum dotnet_methodinvoke_flags
{
    dotnet_methodinvoke_exception_catch = 0,
    dotnet_methodinvoke_exception_flowthrough = 1,
};

typedef dotnet_error (*dotnet_getapi)(const char* apiname, void** pFunctions, int bufferSizeInBytes);

typedef dotnet_error(*_dotnet_uint32_out_voidptr)(uint32_t,void**);
typedef dotnet_error(*_dotnet_voidptr)(void*);
typedef dotnet_error(*_dotnet_object_out_gchandle)(dotnet_object, dotnet_gchandle*);
typedef dotnet_error(*_dotnet_gchandle);
typedef dotnet_error(*_dotnet_gchandle_out_int8)(dotnet_gchandle, int8_t*);
typedef dotnet_error(*_dotnet_gchandle_out_object)(dotnet_gchandle, dotnet_object*);
typedef dotnet_error(*_dotnet_gchandle_object)(dotnet_gchandle, dotnet_object);
typedef dotnet_error(*_dotnet_gchandle_gchandle_out_int8)(dotnet_gchandle,dotnet_gchandle, int8_t*);
typedef dotnet_error(*_dotnet_gchandle_object_out_int8)(dotnet_gchandle,dotnet_object, int8_t*);
typedef dotnet_error(*_out_dotnet_frame)(dotnet_frame *);
typedef dotnet_error(*_dotnet_frame)(dotnet_frame);
typedef dotnet_error(*_dotnet_frame_object)(dotnet_frame,dotnet_object);
typedef dotnet_error(*_dotnet_object_object_out_int8)(dotnet_object,dotnet_object,int8_t*);
typedef dotnet_error(*_dotnet_frame_object_out_object)(dotnet_frame,dotnet_object,dotnet_object*);
typedef dotnet_error(*_dotnet_frame_object_out_typeid)(dotnet_frame,dotnet_object,dotnet_typeid*);
typedef dotnet_error(*_dotnet_frame_object_out_pin_out_voidptr)(dotnet_frame,dotnet_object,dotnet_pin*,void**);
typedef dotnet_error(*_dotnet_frame_pin)(dotnet_frame,dotnet_pin);
typedef dotnet_error(*_dotnet_frame_utf8str_out_object)(dotnet_frame,const char *,dotnet_object*);
typedef dotnet_error(*_dotnet_frame_object_utf8str_bindingflags_objectptr_int32_out_method)(dotnet_frame, dotnet_object, const char*, dotnet_bindingflags, dotnet_object*, int32_t, dotnet_object*);
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
typedef dotnet_error(*_dotnet_togglerefgroup_object_out_toggleref)(dotnet_togglerefgroup, dotnet_toggleref*);
typedef dotnet_error(*_dotnet_togglerefgroup_toggleref)(dotnet_togglerefgroup, dotnet_toggleref);
typedef dotnet_error(*_dotnet_frame_togglerefgroup_toggleref_out_object)(dotnet_frame, dotnet_togglerefgroup, dotnet_toggleref, dotnet_object*);

#define DOTNET_V1_API_GROUP "DOTNET.0"
struct dotnet_embedding_api_group
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
    _dotnet_object_object_out_int8 object_reference_equal;
    _dotnet_frame_object_out_object object_tostring;
    _dotnet_frame_object_out_object object_gettype;
    _dotnet_frame_object_out_typeid object_gettypeid;
 
    // GCHandle functions
    _dotnet_object_out_gchandle gchandle_alloc;
    _dotnet_object_out_gchandle gchandle_alloc_weak;
    _dotnet_object_out_gchandle gchandle_alloc_weak_track_resurrection;
    _dotnet_gchandle gchandle_free;
    _dotnet_gchandle_out_object gchandle_get_target;
    _dotnet_gchandle_object gchandle_set_target;
    _dotnet_gchandle_gchandle_out_int8 gchandle_reference_equal;
    _dotnet_gchandle_object_out_int8 gchandle_reference_equal_object;
    _dotnet_gchandle_out_int8 gchandle_isnull;

    // Reflection surface
    _dotnet_frame_utf8str_out_object type_gettype;
    _dotnet_frame_object_utf8str_bindingflags_objectptr_int32_out_method type_getmethod;

    // Non-managed type system structure access
    _dotnet_object_out_typeid get_typeid;
    _dotnet_object_out_methodid get_methodid;
    _dotnet_object_out_fieldid get_fieldid;
    _dotnet_fieldid_out_typeid get_field_typeid;
    _dotnet_frame_typeid_out_object get_typeid_lifetime_object;
    _dotnet_frame_methodid_out_object get_methodid_lifetime_object;
    
    // Object manipulation
    _dotnet_frame_object_fieldid_voidptr_int32 read_field;
    _dotnet_object_fieldid_voidptr_int32 write_field;
    _dotnet_frame_fieldid_voidptr_int32 read_static_field;
    _dotnet_fieldid_voidptr_int32 write_static_field;
    _dotnet_voidptr_fieldid_voidptr_int32 read_field_on_struct;
    _dotnet_voidptr_fieldid_voidptr_int32 write_field_on_struct;

    // String api
    _dotnet_frame_utf8str_out_object string_alloc_utf8;

    // Method Invoke
    _dotnet_frame_methodid_invokeargumentptr_int32_methodinvokeflags method_invoke;

    // Toggle refs
    _dotnet_togglerefcallback_out_togglerefgroup toggleref_creategroup;
    _dotnet_togglerefcallback_out_togglerefgroup toggleref_destroygroup;
    _dotnet_togglerefgroup_object_out_toggleref toggleref_alloc;
    _dotnet_togglerefgroup_toggleref toggleref_free;
    _dotnet_frame_togglerefgroup_toggleref_out_object toggleref_get_target;
};

#endif // __DOTNET_EMBEDDING_H__