// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

namespace System.Runtime.CompilerServices
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    internal sealed class SupportsEagerFinalizationAttribute : Attribute
    {
        private bool eagerFinalizationBehaviorInherited;

        public SupportsEagerFinalizationAttribute(bool eagerFinalizationBehaviorInherited)
        {
            this.eagerFinalizationBehaviorInherited = eagerFinalizationBehaviorInherited;
        }
    }
}
