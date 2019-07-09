// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//

using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.StubHelpers;
using System.Text;

namespace System.Runtime.InteropServices
{
    unsafe class EmbeddingApi
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr nAllocHandle(IntPtr frame, object? obj);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object? nGetTarget(IntPtr objHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void nPopFrame(IntPtr frame);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr nPushFrame();


        private delegate int Object_ToStringDelegate(IntPtr frame, IntPtr objectHandle, IntPtr *stringHandle);
        private static int Object_ToString(IntPtr frame, IntPtr objectHandle, IntPtr* stringHandle)
        {
            try
            {
                object? o = nGetTarget(objectHandle);
                string? s = o!.ToString();
                *stringHandle = nAllocHandle(frame, s);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        private delegate int Type_GetTypeDelegate(IntPtr frame, IntPtr utf8Str, IntPtr* result);
        private static int Type_GetType(IntPtr frame, IntPtr utf8Str, IntPtr* result)
        {
            try
            {
                string? s = UTF8Marshaler.ConvertToManaged(utf8Str);
                if (s == null)
                    throw new ArgumentNullException("str");
                Type? t = Type.GetType(s);
                *result = nAllocHandle(frame, t);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        private delegate int Type_GetMethodDelegate(IntPtr frame, IntPtr typeHandle, IntPtr methodNameUtf8, BindingFlags flags, IntPtr* types, int cTypes, IntPtr *methodHandle);
        private static int Type_GetMethod(IntPtr frame, IntPtr typeHandle, IntPtr methodNameUtf8, BindingFlags flags, IntPtr* types, int cTypes, IntPtr *methodHandle)
        {
            try
            {
                Type? type = (Type?)nGetTarget(typeHandle);
                if (type == null)
                    throw new ArgumentNullException("type");

                string? methodName = UTF8Marshaler.ConvertToManaged(methodNameUtf8);
                if (methodName == null)
                    throw new ArgumentNullException("methodName");

                Type[] typeOfArguments = new Type[cTypes];
                for (int i = 0; i < cTypes; i++)
                {
                    Type? argumentType = (Type?)nGetTarget(types[i]);
                    if (argumentType == null)
                        throw new ArgumentNullException("types");
                    typeOfArguments[i] = argumentType;
                }
                MethodInfo? method = type.GetMethod(methodName, flags, null, typeOfArguments, null);
                *methodHandle = nAllocHandle(frame, method);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        private delegate int Type_GetFieldDelegate(IntPtr frame, IntPtr typeHandle, IntPtr fieldNameUtf8, BindingFlags flags, IntPtr *fieldHandle);
        private static int Type_GetField(IntPtr frame, IntPtr typeHandle, IntPtr fieldNameUtf8, BindingFlags flags, IntPtr *fieldHandle)
        {
            try
            {
                Type? type = (Type?)nGetTarget(typeHandle);
                if (type == null)
                    throw new ArgumentNullException("type");

                string? fieldName = UTF8Marshaler.ConvertToManaged(fieldNameUtf8);
                if (fieldName == null)
                    throw new ArgumentNullException("fieldName");

                FieldInfo? field = type.GetField(fieldName, flags);
                *fieldHandle = nAllocHandle(frame, field);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }


        private delegate int Type_GetElementTypeDelegate(IntPtr frame, IntPtr typeHandle, IntPtr* elementTypeHandle);
        private static int Type_GetElementType(IntPtr frame, IntPtr typeHandle, IntPtr* elementTypeHandle)
        {
            try
            {
                Type? type = (Type?)nGetTarget(typeHandle);
                if (type == null)
                    throw new ArgumentNullException("type");

                Type? elementType = type.GetElementType();
                *elementTypeHandle = nAllocHandle(frame, elementType);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        private delegate int Type_GetConstructorDelegate(IntPtr frame, IntPtr typeHandle, BindingFlags flags, IntPtr* types, int cTypes, IntPtr* constructorHandle);
        private static int Type_GetConstructor(IntPtr frame, IntPtr typeHandle, BindingFlags flags, IntPtr* types, int cTypes, IntPtr* constructorHandle)
        {
            try
            {
                Type? type = (Type?)nGetTarget(typeHandle);
                if (type == null)
                    throw new ArgumentNullException("type");

                Type[] typeOfArguments = new Type[cTypes];
                for (int i = 0; i < cTypes; i++)
                {
                    Type? argumentType = (Type?)nGetTarget(types[i]);
                    if (argumentType == null)
                        throw new ArgumentNullException("types");
                    typeOfArguments[i] = argumentType;
                }
                ConstructorInfo? ctor = type.GetConstructor(flags, null, typeOfArguments, null);
                *constructorHandle = nAllocHandle(frame, ctor);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }


        private delegate int String_AllocUtf8Delegate(IntPtr frame, IntPtr utf8Str, IntPtr *result);
        private static int String_AllocUtf8(IntPtr frame, IntPtr utf8Str, IntPtr *result)
        {
            try
            {
                string? str = UTF8Marshaler.ConvertToManaged(utf8Str);
                *result = nAllocHandle(frame, str);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        private delegate int Utf8_GetStringDelegate(IntPtr frame, IntPtr stringHandle, IntPtr* utf8String, int* stringSize);
        private static int Utf8_GetString(IntPtr frame, IntPtr stringHandle, IntPtr* utf8String, int* stringSize)
        {
            try
            {
                String? str = (String?)nGetTarget(stringHandle);

                int size = Encoding.UTF8.GetByteCount(str!) + 1;
                *utf8String = Marshal.AllocCoTaskMem(size);
                str!.GetBytesFromEncoding((byte*)*utf8String, size, Encoding.UTF8);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        private delegate int GetMethodFromMethodIdDelegate(IntPtr frame, IntPtr methodId, IntPtr typeId, IntPtr* result);
        private static int GetMethodFromMethodId(IntPtr frame, IntPtr methodId, IntPtr typeId, IntPtr* result)
        {
            try
            {
                RuntimeType? type = null;
                if (typeId != IntPtr.Zero)
                    type = RuntimeType.GetTypeFromHandleUnsafe(typeId);

                if (methodId == IntPtr.Zero)
                    throw new ArgumentNullException();

                object? method = RuntimeType.GetMethodBase(type, new RuntimeMethodHandleInternal(methodId));
                *result = nAllocHandle(frame, method);
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        private delegate int GetMethodTypeIdDelegate(IntPtr methodHandle, IntPtr* methodId);
        private static int GetMethodTypeId(IntPtr methodId, IntPtr* typeId)
        {
            try
            {
                MethodBase? method = RuntimeType.GetMethodBase(null, new RuntimeMethodHandleInternal(methodId));;
                RuntimeType? type = (RuntimeType?)method!.DeclaringType;
                *typeId = type!.m_handle;
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        private enum GetApiHelperEnum
        {
            // Keep this in sync with the enum in embeddingapi.cpp
            object_tostring,
            type_gettype,
            type_getmethod,
            type_getfield,
            type_get_element_type,
            type_getconstructor,
            string_alloc_utf8,
            utf8_getstring,
            get_method_from_methodid,
            get_method_typeid,
        }

        private static IntPtr GetApi(int helperEnum)
        {
            GetApiHelperEnum helper = (GetApiHelperEnum)helperEnum;
            Delegate? del = null;

            switch (helper)
            {
                case GetApiHelperEnum.object_tostring: del = (Object_ToStringDelegate)Object_ToString; break;
                case GetApiHelperEnum.type_getmethod: del = (Type_GetMethodDelegate)Type_GetMethod; break;
                case GetApiHelperEnum.type_getfield: del = (Type_GetFieldDelegate)Type_GetField; break;
                case GetApiHelperEnum.type_gettype: del = (Type_GetTypeDelegate)Type_GetType; break;
                case GetApiHelperEnum.type_get_element_type: del = (Type_GetElementTypeDelegate)Type_GetElementType; break;
                case GetApiHelperEnum.type_getconstructor: del = (Type_GetConstructorDelegate)Type_GetConstructor; break;
                case GetApiHelperEnum.string_alloc_utf8: del = (String_AllocUtf8Delegate)String_AllocUtf8; break;
                case GetApiHelperEnum.utf8_getstring: del = (Utf8_GetStringDelegate)Utf8_GetString; break;
                case GetApiHelperEnum.get_method_from_methodid: del = (GetMethodFromMethodIdDelegate)GetMethodFromMethodId; break;
                case GetApiHelperEnum.get_method_typeid: del = (GetMethodTypeIdDelegate)GetMethodTypeId; break;
            }

            if (del != null)
            {
                GCHandle.Alloc(del);
                return Marshal.GetFunctionPointerForDelegate(del);
            }
            else
                return IntPtr.Zero;
        }
    }
}
