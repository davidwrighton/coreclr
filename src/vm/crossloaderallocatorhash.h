// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef CROSSLOADERALLOCATORHASH_H
#define CROSSLOADERALLOCATORHASH_H

#ifndef CROSSGEN_COMPILE
#include "gcheaphashtable.h"
#endif // !CROSSGEN_COMPILE

class LoaderAllocator;

template <class TKey_, class TValue_>
class NoRemoveDefaultCrossLoaderAllocatorHashTraits
{
public:
    typedef typename TKey_ TKey;
    typedef typename TValue_ TValue;
    typedef SArray<TValue> TLALocalValueStore;

#ifndef DACCESS_COMPILE
    static void AddToValues(SArray<TValue> *pValues, const TValue& value);
#ifndef CROSSGEN_COMPILE
    static void AddToValuesInHeapMemory(OBJECTREF &keyValueStore, OBJECTREF &newKeyValueStore, const TKey& key, const TValue& value);
#endif // !CROSSGEN_COMPILE
#endif //!DACCESS_COMPILE
};

template <class TKey_, class TValue_>
class DefaultCrossLoaderAllocatorHashTraits : public NoRemoveDefaultCrossLoaderAllocatorHashTraits<TKey_, TValue_>
{
public:
#ifndef DACCESS_COMPILE
    static void DeleteValue(SArray<TValue> *pValues, const TValue& value);
#ifndef CROSSGEN_COMPILE
    static void DeleteValueInHeapMemory(OBJECTREF keyValueStore, const TValue& value);
#endif // !CROSSGEN_COMPILE
#endif //!DACCESS_COMPILE
};

#ifndef CROSSGEN_COMPILE
struct GCHeapHashDependentHashTrackerHashTraits : public DefaultGCHeapHashTraits<true>
{
    typedef LoaderAllocator* PtrTypeKey;
    
    static INT32 Hash(PtrTypeKey *pValue);
    static INT32 Hash(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index);
    static bool DoesEntryMatchKey(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, PtrTypeKey *pKey);
    static bool IsDeleted(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index);
};

typedef GCHeapHash<GCHeapHashDependentHashTrackerHashTraits> GCHeapHashDependentHashTrackerHash;

struct GCHeapHashKeyToDependentTrackersHashTraits : public DefaultGCHeapHashTraits<false>
{
    template <class TKey>
    static INT32 Hash(TKey *pValue);
    static INT32 Hash(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index);

    template<class TKey>
    static bool DoesEntryMatchKey(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, TKey *pKey);
};

typedef GCHeapHash<GCHeapHashKeyToDependentTrackersHashTraits> GCHeapHashKeyToDependentTrackersHash;
#endif // !CROSSGEN_COMPILE

template <class TKey, class TValue, class TLALocalValueStore>
struct WithinLoaderAllocatorData
{
    mutable TKey _key;
    mutable TLALocalValueStore _values;
};

// Hashtable of pointer to pointer where the key may live in a different loader allocator than the value
// and this should not keep the loaderallocator of Key alive.
template <class TRAITS>
class CrossLoaderAllocatorHash
{
private:
    typedef typename TRAITS::TKey TKey;
    typedef typename TRAITS::TValue TValue;
    typedef typename TRAITS::TLALocalValueStore TLALocalValueStore;

    typedef typename WithinLoaderAllocatorData<TKey, TValue, TLALocalValueStore> WithinLoaderAllocatorDataStruct;

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

    // Initialize this CrossLoaderAllocatorHash to be associated with a specific LoaderAllocator
    // Must be called before any use of Add
    void Init(LoaderAllocator *pAssociatedLoaderAllocator);

private:
    class WithinLoaderAllocatorDataHashTraits : public DeleteElementsOnDestructSHashTraits<NoRemoveSHashTraits<DefaultSHashTraits<WithinLoaderAllocatorDataStruct>>>
    {
    public:
        typedef DeleteElementsOnDestructSHashTraits<NoRemoveSHashTraits<DefaultSHashTraits<WithinLoaderAllocatorDataStruct>>> Base;
        typedef COUNT_T count_t;

        typedef TKey key_t;

        void OnDestructPerEntryCleanupAction(const WithinLoaderAllocatorDataStruct & elem)
        {
            WRAPPER_NO_CONTRACT;

            elem.~WithinLoaderAllocatorData();
            elem._key = nullptr;
        }
        static const bool s_DestructPerEntryCleanupAction = true;

        static key_t GetKey(const WithinLoaderAllocatorDataStruct &e)
        {
            LIMITED_METHOD_CONTRACT;
            return (key_t)((INT_PTR)e._key & ~1);
        }

        static BOOL Equals(key_t k1, key_t k2)
        {
            LIMITED_METHOD_CONTRACT;
            return k1 == k2;
        }

        static count_t Hash(key_t k)
        {
            LIMITED_METHOD_CONTRACT;
            return (count_t)((size_t)dac_cast<TADDR>(k) >> 2);
        }

        static const WithinLoaderAllocatorDataStruct Null() { LIMITED_METHOD_CONTRACT; WithinLoaderAllocatorDataStruct nullValue = {0}; return nullValue; }
        static bool IsNull(const WithinLoaderAllocatorDataStruct &e) { LIMITED_METHOD_CONTRACT; return e._key == nullptr; }
    };

    typedef SHash<WithinLoaderAllocatorDataHashTraits> WithinLoaderAllocatorDataHash;
#ifndef CROSSGEN_COMPILE
    typedef GCHeapHash<GCHeapHashTraitsPointerToPointerList<TKey, false>> KeyToValuesGCHeapHash;
#endif // !CROSSGEN_COMPILE

#ifndef CROSSGEN_COMPILE
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
#endif // !CROSSGEN_COMPILE

#ifndef CROSSGEN_COMPILE
    void EnsureManagedObjectsInitted();
    LAHASHDEPENDENTHASHTRACKERREF GetDependentTrackerForLoaderAllocator(LoaderAllocator* pLoaderAllocator);
    GCHEAPHASHOBJECTREF GetKeyToValueCrossLAHash(TKey key, LoaderAllocator* pValueLoaderAllocator);
    template <class Visitor>
    static bool VisitTracker(TKey key, LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe, Visitor &visitor);
    static void DeleteEntryTracker(TKey key, LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe);
#endif // !CROSSGEN_COMPILE

private:
    WithinLoaderAllocatorDataHash _withinLAHash;
    LoaderAllocator *_loaderAllocator = 0;
    LOADERHANDLE LAToDependentTrackerHash = 0;
    LOADERHANDLE KeyToDependentTrackersHash = 0;
};

#endif // CROSSLOADERALLOCATORHASH_H
