//
// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//

#ifndef __CRST_TYPES_INCLUDED
#define __CRST_TYPES_INCLUDED

// **** THIS IS AN AUTOMATICALLY GENERATED HEADER FILE -- DO NOT EDIT!!! ****

// This file describes the range of Crst types available and their mapping to a numeric level (used by the
// runtime in debug mode to validate we're deadlock free). To modify these settings edit the
// file:CrstTypes.def file and run the clr\bin\CrstTypeTool utility to generate a new version of this file.

// Each Crst type is declared as a value in the following CrstType enum.
enum CrstType
{
    CrstActiveLoaderAllocators = 0,
    CrstAllowedFiles = 1,
    CrstAppDomainCache = 2,
    CrstAppDomainHandleTable = 3,
    CrstArgBasedStubCache = 4,
    CrstAssemblyDependencyGraph = 5,
    CrstAssemblyIdentityCache = 6,
    CrstAssemblyList = 7,
    CrstAssemblyLoader = 8,
    CrstAvailableClass = 9,
    CrstAvailableParamTypes = 10,
    CrstBaseDomain = 11,
    CrstCCompRC = 12,
    CrstCer = 13,
    CrstClassFactInfoHash = 14,
    CrstClassInit = 15,
    CrstClrNotification = 16,
    CrstCLRPrivBinderMaps = 17,
    CrstCLRPrivBinderMapsAdd = 18,
    CrstCodeFragmentHeap = 19,
    CrstCOMCallWrapper = 20,
    CrstCOMWrapperCache = 21,
    CrstConnectionNameTable = 22,
    CrstContexts = 23,
    CrstCoreCLRBinderLog = 24,
    CrstCrstCLRPrivBinderLocalWinMDPath = 25,
    CrstCSPCache = 26,
    CrstDataTest1 = 27,
    CrstDataTest2 = 28,
    CrstDbgTransport = 29,
    CrstDeadlockDetection = 30,
    CrstDebuggerController = 31,
    CrstDebuggerFavorLock = 32,
    CrstDebuggerHeapExecMemLock = 33,
    CrstDebuggerHeapLock = 34,
    CrstDebuggerJitInfo = 35,
    CrstDebuggerMutex = 36,
    CrstDelegateToFPtrHash = 37,
    CrstDomainLocalBlock = 38,
    CrstDynamicIL = 39,
    CrstDynamicMT = 40,
    CrstDynLinkZapItems = 41,
    CrstEtwTypeLogHash = 42,
    CrstEventPipe = 43,
    CrstEventStore = 44,
    CrstException = 45,
    CrstExecuteManLock = 46,
    CrstExecuteManRangeLock = 47,
    CrstFCall = 48,
    CrstFriendAccessCache = 49,
    CrstFuncPtrStubs = 50,
    CrstFusionAppCtx = 51,
    CrstGCCover = 52,
    CrstGCMemoryPressure = 53,
    CrstGlobalStrLiteralMap = 54,
    CrstHandleTable = 55,
    CrstHostAssemblyMap = 56,
    CrstHostAssemblyMapAdd = 57,
    CrstIbcProfile = 58,
    CrstIJWFixupData = 59,
    CrstIJWHash = 60,
    CrstILStubGen = 61,
    CrstInlineTrackingMap = 62,
    CrstInstMethodHashTable = 63,
    CrstInterfaceVTableMap = 64,
    CrstInterop = 65,
    CrstInteropData = 66,
    CrstIOThreadpoolWorker = 67,
    CrstIsJMCMethod = 68,
    CrstISymUnmanagedReader = 69,
    CrstJit = 70,
    CrstJitGenericHandleCache = 71,
    CrstJitPerf = 72,
    CrstJumpStubCache = 73,
    CrstLeafLock = 74,
    CrstListLock = 75,
    CrstLoaderAllocator = 76,
    CrstLoaderAllocatorReferences = 77,
    CrstLoaderHeap = 78,
    CrstMda = 79,
    CrstMetadataTracker = 80,
    CrstMethodDescBackpatchInfoTracker = 81,
    CrstModIntPairList = 82,
    CrstModule = 83,
    CrstModuleFixup = 84,
    CrstModuleLookupTable = 85,
    CrstMulticoreJitHash = 86,
    CrstMulticoreJitManager = 87,
    CrstMUThunkHash = 88,
    CrstNativeBinderInit = 89,
    CrstNativeImageCache = 90,
    CrstNls = 91,
    CrstNotifyGdb = 92,
    CrstObjectList = 93,
    CrstOnEventManager = 94,
    CrstPatchEntryPoint = 95,
    CrstPEImage = 96,
    CrstPEImagePDBStream = 97,
    CrstPendingTypeLoadEntry = 98,
    CrstPinHandle = 99,
    CrstPinnedByrefValidation = 100,
    CrstProfilerGCRefDataFreeList = 101,
    CrstProfilingAPIStatus = 102,
    CrstPublisherCertificate = 103,
    CrstRCWCache = 104,
    CrstRCWCleanupList = 105,
    CrstRCWRefCache = 106,
    CrstReadyToRunEntryPointToMethodDescMap = 107,
    CrstReDacl = 108,
    CrstReflection = 109,
    CrstReJITDomainTable = 110,
    CrstReJITGlobalRequest = 111,
    CrstRemoting = 112,
    CrstRetThunkCache = 113,
    CrstRWLock = 114,
    CrstSavedExceptionInfo = 115,
    CrstSaveModuleProfileData = 116,
    CrstSecurityStackwalkCache = 117,
    CrstSharedAssemblyCreate = 118,
    CrstSigConvert = 119,
    CrstSingleUseLock = 120,
    CrstSpecialStatics = 121,
    CrstSqmManager = 122,
    CrstStackSampler = 123,
    CrstStressLog = 124,
    CrstStrongName = 125,
    CrstStubCache = 126,
    CrstStubDispatchCache = 127,
    CrstStubUnwindInfoHeapSegments = 128,
    CrstSyncBlockCache = 129,
    CrstSyncHashLock = 130,
    CrstSystemBaseDomain = 131,
    CrstSystemDomain = 132,
    CrstSystemDomainDelayedUnloadList = 133,
    CrstThreadIdDispenser = 134,
    CrstThreadpoolEventCache = 135,
    CrstThreadpoolTimerQueue = 136,
    CrstThreadpoolWaitThreads = 137,
    CrstThreadpoolWorker = 138,
    CrstThreadStaticDataHashTable = 139,
    CrstThreadStore = 140,
    CrstTieredCompilation = 141,
    CrstTPMethodTable = 142,
    CrstTypeEquivalenceMap = 143,
    CrstTypeIDMap = 144,
    CrstUMEntryThunkCache = 145,
    CrstUMThunkHash = 146,
    CrstUniqueStack = 147,
    CrstUnresolvedClassLock = 148,
    CrstUnwindInfoTableLock = 149,
    CrstVSDIndirectionCellLock = 150,
    CrstWinRTFactoryCache = 151,
    CrstWrapperTemplate = 152,
    kNumberOfCrstTypes = 153
};

#endif // __CRST_TYPES_INCLUDED

// Define some debug data in one module only -- vm\crst.cpp.
#if defined(__IN_CRST_CPP) && defined(_DEBUG)

// An array mapping CrstType to level.
int g_rgCrstLevelMap[] =
{
    16,         // CrstActiveLoaderAllocators
    9,          // CrstAllowedFiles
    9,          // CrstAppDomainCache
    14,         // CrstAppDomainHandleTable
    0,          // CrstArgBasedStubCache
    0,          // CrstAssemblyDependencyGraph
    0,          // CrstAssemblyIdentityCache
    0,          // CrstAssemblyList
    7,          // CrstAssemblyLoader
    3,          // CrstAvailableClass
    3,          // CrstAvailableParamTypes
    7,          // CrstBaseDomain
    -1,         // CrstCCompRC
    9,          // CrstCer
    13,         // CrstClassFactInfoHash
    8,          // CrstClassInit
    -1,         // CrstClrNotification
    0,          // CrstCLRPrivBinderMaps
    3,          // CrstCLRPrivBinderMapsAdd
    6,          // CrstCodeFragmentHeap
    0,          // CrstCOMCallWrapper
    4,          // CrstCOMWrapperCache
    0,          // CrstConnectionNameTable
    17,         // CrstContexts
    -1,         // CrstCoreCLRBinderLog
    0,          // CrstCrstCLRPrivBinderLocalWinMDPath
    7,          // CrstCSPCache
    3,          // CrstDataTest1
    0,          // CrstDataTest2
    0,          // CrstDbgTransport
    0,          // CrstDeadlockDetection
    -1,         // CrstDebuggerController
    3,          // CrstDebuggerFavorLock
    0,          // CrstDebuggerHeapExecMemLock
    0,          // CrstDebuggerHeapLock
    4,          // CrstDebuggerJitInfo
    11,         // CrstDebuggerMutex
    0,          // CrstDelegateToFPtrHash
    16,         // CrstDomainLocalBlock
    0,          // CrstDynamicIL
    3,          // CrstDynamicMT
    3,          // CrstDynLinkZapItems
    7,          // CrstEtwTypeLogHash
    19,         // CrstEventPipe
    0,          // CrstEventStore
    0,          // CrstException
    7,          // CrstExecuteManLock
    0,          // CrstExecuteManRangeLock
    3,          // CrstFCall
    7,          // CrstFriendAccessCache
    7,          // CrstFuncPtrStubs
    5,          // CrstFusionAppCtx
    11,         // CrstGCCover
    0,          // CrstGCMemoryPressure
    13,         // CrstGlobalStrLiteralMap
    1,          // CrstHandleTable
    0,          // CrstHostAssemblyMap
    3,          // CrstHostAssemblyMapAdd
    0,          // CrstIbcProfile
    9,          // CrstIJWFixupData
    0,          // CrstIJWHash
    7,          // CrstILStubGen
    3,          // CrstInlineTrackingMap
    17,         // CrstInstMethodHashTable
    0,          // CrstInterfaceVTableMap
    19,         // CrstInterop
    4,          // CrstInteropData
    13,         // CrstIOThreadpoolWorker
    0,          // CrstIsJMCMethod
    7,          // CrstISymUnmanagedReader
    8,          // CrstJit
    0,          // CrstJitGenericHandleCache
    -1,         // CrstJitPerf
    6,          // CrstJumpStubCache
    0,          // CrstLeafLock
    -1,         // CrstListLock
    15,         // CrstLoaderAllocator
    16,         // CrstLoaderAllocatorReferences
    0,          // CrstLoaderHeap
    0,          // CrstMda
    -1,         // CrstMetadataTracker
    9,          // CrstMethodDescBackpatchInfoTracker
    0,          // CrstModIntPairList
    4,          // CrstModule
    15,         // CrstModuleFixup
    3,          // CrstModuleLookupTable
    0,          // CrstMulticoreJitHash
    13,         // CrstMulticoreJitManager
    0,          // CrstMUThunkHash
    -1,         // CrstNativeBinderInit
    -1,         // CrstNativeImageCache
    0,          // CrstNls
    0,          // CrstNotifyGdb
    2,          // CrstObjectList
    0,          // CrstOnEventManager
    0,          // CrstPatchEntryPoint
    4,          // CrstPEImage
    0,          // CrstPEImagePDBStream
    18,         // CrstPendingTypeLoadEntry
    0,          // CrstPinHandle
    0,          // CrstPinnedByrefValidation
    0,          // CrstProfilerGCRefDataFreeList
    0,          // CrstProfilingAPIStatus
    0,          // CrstPublisherCertificate
    3,          // CrstRCWCache
    0,          // CrstRCWCleanupList
    3,          // CrstRCWRefCache
    4,          // CrstReadyToRunEntryPointToMethodDescMap
    0,          // CrstReDacl
    9,          // CrstReflection
    10,         // CrstReJITDomainTable
    14,         // CrstReJITGlobalRequest
    19,         // CrstRemoting
    3,          // CrstRetThunkCache
    0,          // CrstRWLock
    3,          // CrstSavedExceptionInfo
    0,          // CrstSaveModuleProfileData
    0,          // CrstSecurityStackwalkCache
    4,          // CrstSharedAssemblyCreate
    3,          // CrstSigConvert
    5,          // CrstSingleUseLock
    0,          // CrstSpecialStatics
    0,          // CrstSqmManager
    0,          // CrstStackSampler
    -1,         // CrstStressLog
    0,          // CrstStrongName
    5,          // CrstStubCache
    0,          // CrstStubDispatchCache
    4,          // CrstStubUnwindInfoHeapSegments
    3,          // CrstSyncBlockCache
    0,          // CrstSyncHashLock
    4,          // CrstSystemBaseDomain
    13,         // CrstSystemDomain
    0,          // CrstSystemDomainDelayedUnloadList
    0,          // CrstThreadIdDispenser
    0,          // CrstThreadpoolEventCache
    7,          // CrstThreadpoolTimerQueue
    7,          // CrstThreadpoolWaitThreads
    13,         // CrstThreadpoolWorker
    4,          // CrstThreadStaticDataHashTable
    12,         // CrstThreadStore
    9,          // CrstTieredCompilation
    9,          // CrstTPMethodTable
    3,          // CrstTypeEquivalenceMap
    7,          // CrstTypeIDMap
    3,          // CrstUMEntryThunkCache
    0,          // CrstUMThunkHash
    3,          // CrstUniqueStack
    7,          // CrstUnresolvedClassLock
    3,          // CrstUnwindInfoTableLock
    3,          // CrstVSDIndirectionCellLock
    3,          // CrstWinRTFactoryCache
    3,          // CrstWrapperTemplate
};

// An array mapping CrstType to a stringized name.
LPCSTR g_rgCrstNameMap[] =
{
    "CrstActiveLoaderAllocators",
    "CrstAllowedFiles",
    "CrstAppDomainCache",
    "CrstAppDomainHandleTable",
    "CrstArgBasedStubCache",
    "CrstAssemblyDependencyGraph",
    "CrstAssemblyIdentityCache",
    "CrstAssemblyList",
    "CrstAssemblyLoader",
    "CrstAvailableClass",
    "CrstAvailableParamTypes",
    "CrstBaseDomain",
    "CrstCCompRC",
    "CrstCer",
    "CrstClassFactInfoHash",
    "CrstClassInit",
    "CrstClrNotification",
    "CrstCLRPrivBinderMaps",
    "CrstCLRPrivBinderMapsAdd",
    "CrstCodeFragmentHeap",
    "CrstCOMCallWrapper",
    "CrstCOMWrapperCache",
    "CrstConnectionNameTable",
    "CrstContexts",
    "CrstCoreCLRBinderLog",
    "CrstCrstCLRPrivBinderLocalWinMDPath",
    "CrstCSPCache",
    "CrstDataTest1",
    "CrstDataTest2",
    "CrstDbgTransport",
    "CrstDeadlockDetection",
    "CrstDebuggerController",
    "CrstDebuggerFavorLock",
    "CrstDebuggerHeapExecMemLock",
    "CrstDebuggerHeapLock",
    "CrstDebuggerJitInfo",
    "CrstDebuggerMutex",
    "CrstDelegateToFPtrHash",
    "CrstDomainLocalBlock",
    "CrstDynamicIL",
    "CrstDynamicMT",
    "CrstDynLinkZapItems",
    "CrstEtwTypeLogHash",
    "CrstEventPipe",
    "CrstEventStore",
    "CrstException",
    "CrstExecuteManLock",
    "CrstExecuteManRangeLock",
    "CrstFCall",
    "CrstFriendAccessCache",
    "CrstFuncPtrStubs",
    "CrstFusionAppCtx",
    "CrstGCCover",
    "CrstGCMemoryPressure",
    "CrstGlobalStrLiteralMap",
    "CrstHandleTable",
    "CrstHostAssemblyMap",
    "CrstHostAssemblyMapAdd",
    "CrstIbcProfile",
    "CrstIJWFixupData",
    "CrstIJWHash",
    "CrstILStubGen",
    "CrstInlineTrackingMap",
    "CrstInstMethodHashTable",
    "CrstInterfaceVTableMap",
    "CrstInterop",
    "CrstInteropData",
    "CrstIOThreadpoolWorker",
    "CrstIsJMCMethod",
    "CrstISymUnmanagedReader",
    "CrstJit",
    "CrstJitGenericHandleCache",
    "CrstJitPerf",
    "CrstJumpStubCache",
    "CrstLeafLock",
    "CrstListLock",
    "CrstLoaderAllocator",
    "CrstLoaderAllocatorReferences",
    "CrstLoaderHeap",
    "CrstMda",
    "CrstMetadataTracker",
    "CrstMethodDescBackpatchInfoTracker",
    "CrstModIntPairList",
    "CrstModule",
    "CrstModuleFixup",
    "CrstModuleLookupTable",
    "CrstMulticoreJitHash",
    "CrstMulticoreJitManager",
    "CrstMUThunkHash",
    "CrstNativeBinderInit",
    "CrstNativeImageCache",
    "CrstNls",
    "CrstNotifyGdb",
    "CrstObjectList",
    "CrstOnEventManager",
    "CrstPatchEntryPoint",
    "CrstPEImage",
    "CrstPEImagePDBStream",
    "CrstPendingTypeLoadEntry",
    "CrstPinHandle",
    "CrstPinnedByrefValidation",
    "CrstProfilerGCRefDataFreeList",
    "CrstProfilingAPIStatus",
    "CrstPublisherCertificate",
    "CrstRCWCache",
    "CrstRCWCleanupList",
    "CrstRCWRefCache",
    "CrstReadyToRunEntryPointToMethodDescMap",
    "CrstReDacl",
    "CrstReflection",
    "CrstReJITDomainTable",
    "CrstReJITGlobalRequest",
    "CrstRemoting",
    "CrstRetThunkCache",
    "CrstRWLock",
    "CrstSavedExceptionInfo",
    "CrstSaveModuleProfileData",
    "CrstSecurityStackwalkCache",
    "CrstSharedAssemblyCreate",
    "CrstSigConvert",
    "CrstSingleUseLock",
    "CrstSpecialStatics",
    "CrstSqmManager",
    "CrstStackSampler",
    "CrstStressLog",
    "CrstStrongName",
    "CrstStubCache",
    "CrstStubDispatchCache",
    "CrstStubUnwindInfoHeapSegments",
    "CrstSyncBlockCache",
    "CrstSyncHashLock",
    "CrstSystemBaseDomain",
    "CrstSystemDomain",
    "CrstSystemDomainDelayedUnloadList",
    "CrstThreadIdDispenser",
    "CrstThreadpoolEventCache",
    "CrstThreadpoolTimerQueue",
    "CrstThreadpoolWaitThreads",
    "CrstThreadpoolWorker",
    "CrstThreadStaticDataHashTable",
    "CrstThreadStore",
    "CrstTieredCompilation",
    "CrstTPMethodTable",
    "CrstTypeEquivalenceMap",
    "CrstTypeIDMap",
    "CrstUMEntryThunkCache",
    "CrstUMThunkHash",
    "CrstUniqueStack",
    "CrstUnresolvedClassLock",
    "CrstUnwindInfoTableLock",
    "CrstVSDIndirectionCellLock",
    "CrstWinRTFactoryCache",
    "CrstWrapperTemplate",
};

// Define a special level constant for unordered locks.
#define CRSTUNORDERED (-1)

// Define inline helpers to map Crst types to names and levels.
inline static int GetCrstLevel(CrstType crstType)
{
    LIMITED_METHOD_CONTRACT;
    _ASSERTE(crstType >= 0 && crstType < kNumberOfCrstTypes);
    return g_rgCrstLevelMap[crstType];
}
inline static LPCSTR GetCrstName(CrstType crstType)
{
    LIMITED_METHOD_CONTRACT;
    _ASSERTE(crstType >= 0 && crstType < kNumberOfCrstTypes);
    return g_rgCrstNameMap[crstType];
}

#endif // defined(__IN_CRST_CPP) && defined(_DEBUG)
