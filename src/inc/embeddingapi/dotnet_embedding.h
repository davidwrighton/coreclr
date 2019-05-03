// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __DOTNET_EMBEDDING_H__
#define __DOTNET_EMBEDDING_H__

#include <stdint.h>

struct _struct_dotnet_object {};
typedef _struct_dotnet_object* dotnet_object;
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

typedef uint32_t dotnet_error;

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
};


#endif // __DOTNET_EMBEDDING_H__