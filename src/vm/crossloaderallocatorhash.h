// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef CROSSLOADERALLOCATORHASH_H
#define CROSSLOADERALLOCATORHASH_H
#ifndef CROSSGEN_COMPILE

#include "gcheaphashtable.h"

class LoaderAllocator;

template <class TKey_, class TValue_>
class NoRemoveDefaultCrossLoaderAllocatorHashTraits
{
public:
    typedef typename TKey_ TKey;
    typedef typename TValue_ TValue;

#ifndef DACCESS_COMPILE
    static void SetUsedEntries(TValue* pStartOfValuesData, DWORD entriesInArrayTotal, DWORD usedEntries);
    static void AddToValuesInHeapMemory(OBJECTREF &keyValueStore, OBJECTREF &newKeyValueStore, const TKey& key, const TValue& value);
#endif //!DACCESS_COMPILE
    static DWORD ComputeUsedEntries(OBJECTREF &keyValueStore, DWORD *pEntriesInArrayTotal);
    template <class Visitor>
    static bool VisitKeyValueStore(OBJECTREF *pLoaderAllocatorRef, OBJECTREF *pKeyValueStore, Visitor &visitor);
    static TKey ReadKeyFromKeyValueStore(OBJECTREF *pKeyValueStore);
};

template <class TKey_, class TValue_>
class DefaultCrossLoaderAllocatorHashTraits : public NoRemoveDefaultCrossLoaderAllocatorHashTraits<TKey_, TValue_>
{
public:
#ifndef DACCESS_COMPILE
    static void DeleteValueInHeapMemory(OBJECTREF keyValueStore, const TValue& value);
#endif //!DACCESS_COMPILE
};

struct GCHeapHashDependentHashTrackerHashTraits : public DefaultGCHeapHashTraits<true>
{
    typedef LoaderAllocator* PtrTypeKey;
    
    static INT32 Hash(PtrTypeKey *pValue);
    static INT32 Hash(PTRARRAYREF arr, INT32 index);
    static bool DoesEntryMatchKey(PTRARRAYREF arr, INT32 index, PtrTypeKey *pKey);
    static bool IsDeleted(PTRARRAYREF arr, INT32 index, GCHEAPHASHOBJECTREF gcHeap);
};

typedef GCHeapHash<GCHeapHashDependentHashTrackerHashTraits> GCHeapHashDependentHashTrackerHash;

template<class TRAITS>
struct KeyToValuesGCHeapHashTraits : public DefaultGCHeapHashTraits<true>
{
    template <class TKey>
    static INT32 Hash(TKey *pValue);
    static INT32 Hash(PTRARRAYREF arr, INT32 index);

    template<class TKey>
    static bool DoesEntryMatchKey(PTRARRAYREF arr, INT32 index, TKey *pKey);
};

// Hashtable of pointer to pointer where the key may live in a different loader allocator than the value
// and this should not keep the loaderallocator of Key alive.
template <class TRAITS>
class CrossLoaderAllocatorHash
{
private:
    typedef typename TRAITS::TKey TKey;
    typedef typename TRAITS::TValue TValue;
    typedef GCHeapHash<KeyToValuesGCHeapHashTraits<TRAITS>> KeyToValuesGCHeapHash;

public:

#ifndef DACCESS_COMPILE
    // Add an entry to the CrossLoaderAllocatorHash
    // key must implement GetLoaderAllocator()
    void Add(TKey key, TValue value, LoaderAllocator *pLoaderAllocatorOfValue);

    // Add an entry to the CrossLoaderAllocatorHash
    // key must implement GetLoaderAllocator()
    void Remove(TKey key, TValue value, LoaderAllocator *pLoaderAllocatorOfValue);

    // Remove all entries that can be looked up by key
    void RemoveAll(TKey key);
#endif

    // Using visitor walk all values associated with a given key. The visitor
    // is expected to implement bool operator ()(OBJECTREF keepAlive, TValue value)
    // The value of "value" in this case must not escape from the visitor object
    // unless the keepAlive OBJECTREF is also kept alive
    template <class Visitor>
    bool VisitValuesOfKey(TKey key, Visitor &visitor);

    // Visit all key/value pairs
    template <class Visitor>
    bool VisitAllKeyValuePairs(Visitor &visitor);

    // Initialize this CrossLoaderAllocatorHash to be associated with a specific LoaderAllocator
    // Must be called before any use of Add
    void Init(LoaderAllocator *pAssociatedLoaderAllocator);

private:
    template <class Visitor>
    class VisitIndividualEntryKeyValueHash
    {
        public:
        TKey _key;
        Visitor *_pVisitor;
        GCHeapHashDependentHashTrackerHash *_pDependentTrackerHash;

        VisitIndividualEntryKeyValueHash(TKey key, Visitor *pVisitor,  GCHeapHashDependentHashTrackerHash *pDependentTrackerHash) : 
            _key(key),
            _pVisitor(pVisitor),
            _pDependentTrackerHash(pDependentTrackerHash)
            {}

        bool operator()(INT32 index)
        {
            WRAPPER_NO_CONTRACT;

            LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
            _pDependentTrackerHash->GetElement(index, dependentTracker);
            return VisitTracker(_key, dependentTracker, *_pVisitor);
        }
    };

    template <class Visitor>
    class VisitAllEntryDependentTrackerHash
    {
        public:
        Visitor *_pVisitor;
        GCHeapHashDependentHashTrackerHash *_pDependentTrackerHash;

        VisitAllEntryDependentTrackerHash(Visitor *pVisitor,  GCHeapHashDependentHashTrackerHash *pDependentTrackerHash) : 
            _pVisitor(pVisitor),
            _pDependentTrackerHash(pDependentTrackerHash)
            {}

        bool operator()(INT32 index)
        {
            WRAPPER_NO_CONTRACT;

            LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
            _pDependentTrackerHash->GetElement(index, dependentTracker);
            return VisitTrackerAllEntries(dependentTracker, *_pVisitor);
        }
    };

    template <class Visitor>
    class VisitAllEntryKeyToDependentTrackerHash
    {
        public:
        Visitor *_pVisitor;
        KeyToValuesGCHeapHash *_pKeyToTrackerHash;

        VisitAllEntryKeyToDependentTrackerHash(Visitor *pVisitor,  KeyToValuesGCHeapHash *pKeyToTrackerHash) : 
            _pVisitor(pVisitor),
            _pKeyToTrackerHash(pKeyToTrackerHash)
            {}

        bool operator()(INT32 index)
        {
            WRAPPER_NO_CONTRACT;

            OBJECTREF hashKeyEntry;
            _pKeyToTrackerHash->GetElement(index, hashKeyEntry);
            return VisitKeyToTrackerAllEntries(hashKeyEntry, *_pVisitor);
        }
    };

#ifndef DACCESS_COMPILE
    class DeleteIndividualEntryKeyValueHash
    {
        public:
        TKey _key;
        GCHeapHashDependentHashTrackerHash *_pDependentTrackerHash;

        DeleteIndividualEntryKeyValueHash(TKey key, GCHeapHashDependentHashTrackerHash *pDependentTrackerHash) : 
            _key(key),
            _pDependentTrackerHash(pDependentTrackerHash)
            {}

        bool operator()(INT32 index)
        {
            WRAPPER_NO_CONTRACT;

            LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
            _pDependentTrackerHash->GetElement(index, dependentTracker);
            DeleteEntryTracker(_key, dependentTracker);
            return true;
        }
    };
#endif // !DACCESS_COMPILE

    void EnsureManagedObjectsInitted();
    LAHASHDEPENDENTHASHTRACKERREF GetDependentTrackerForLoaderAllocator(LoaderAllocator* pLoaderAllocator);
    GCHEAPHASHOBJECTREF GetKeyToValueCrossLAHashForHashkeyToTrackers(LAHASHKEYTOTRACKERSREF hashKeyToTrackersUnsafe, LoaderAllocator* pValueLoaderAllocator);

    template <class Visitor>
    static bool VisitKeyValueStore(OBJECTREF *pLoaderAllocatorRef, OBJECTREF *pKeyValueStore, Visitor &visitor);
    template <class Visitor>
    static bool VisitTracker(TKey key, LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe, Visitor &visitor);
    template <class Visitor>
    static bool VisitTrackerAllEntries(LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe, Visitor &visitor);
    template <class Visitor>
    static bool VisitKeyToTrackerAllEntries(OBJECTREF hashKeyEntryUnsafe, Visitor &visitor);
    static void DeleteEntryTracker(TKey key, LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe);

private:
    LoaderAllocator *_loaderAllocator = 0;
    OBJECTHANDLE LAToDependentTrackerHash = 0;
    OBJECTHANDLE KeyToDependentTrackersHash = 0;
};

#endif // !CROSSGEN_COMPILE
#endif // CROSSLOADERALLOCATORHASH_H
