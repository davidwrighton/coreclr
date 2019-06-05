// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "common.h"
#include "gcenv.h"
#include "gcenv.ee.h"
#include "threadsuspend.h"
#include "nativeoverlapped.h"
#include "embeddingapi_impl.h"

#ifdef FEATURE_COMINTEROP
#include "runtimecallablewrapper.h"
#include "rcwwalker.h"
#include "comcallablewrapper.h"
#endif // FEATURE_COMINTEROP

#include "gctoclreventsink.h"

namespace standalone
{

#include "gcenv.ee.cpp"

} // namespace standalone
