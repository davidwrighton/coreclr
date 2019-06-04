// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//

using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.StubHelpers;

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

        private enum GetApiHelperEnum
        {
            // Keep this in sync with the num in embeddingapi_impl.h
            Type_GetType,
            Type_GetMethod,
            String_AllocUtf8
        }

        private static IntPtr GetApi(int helperEnum)
        {
            GetApiHelperEnum helper = (GetApiHelperEnum)helperEnum;
            Delegate? del = null;

            switch (helper)
            {
                case GetApiHelperEnum.Type_GetType: del = (Type_GetTypeDelegate)Type_GetType; break;
                case GetApiHelperEnum.Type_GetMethod: del = (Type_GetMethodDelegate)Type_GetMethod; break;
                case GetApiHelperEnum.String_AllocUtf8: del = (String_AllocUtf8Delegate)String_AllocUtf8; break;
            }

            if (del != null)
                return Marshal.GetFunctionPointerForDelegate(del);
            else
                return IntPtr.Zero;
        }
    }
}