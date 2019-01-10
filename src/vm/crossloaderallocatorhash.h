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
class DefaultCrossLoaderAllocatorHashTraits
{
    typedef typename TKey_ TKey;
    typedef typename TValue_ TValue;
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

template <class TKey, class TValue>
struct WithinLoaderAllocatorData
{
    mutable TKey _key;
    mutable SArray<TValue> _values;
};

// Hashtable of pointer to pointer where the key may live in a different loader allocator than the value
// and this should not keep the loaderallocator of Key alive.
template <class TRAITS>
class CrossLoaderAllocatorHashNoRemove
{
private:
    typedef typename TRAITS::TKey TKey;
    typedef typename TRAITS::TValue TValue;

public:

    // Add an entry to the CrossLoaderAllocatorHashNoRemove
    // key must implement GetLoaderAllocator()
    void Add(TKey key, TValue value, LoaderAllocator *pLoaderAllocatorOfValue);

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
    class WithinLoaderAllocatorDataHashTraits : public DeleteElementsOnDestructSHashTraits<NoRemoveSHashTraits<DefaultSHashTraits<WithinLoaderAllocatorData<TKey, TValue>>>>
    {
    public:
        typedef DeleteElementsOnDestructSHashTraits<NoRemoveSHashTraits<DefaultSHashTraits<WithinLoaderAllocatorData<TKey, TValue>>>> Base;
        typedef COUNT_T count_t;

        typedef TKey key_t;

        void OnDestructPerEntryCleanupAction(const WithinLoaderAllocatorData<TKey, TValue> & elem)
        {
            WRAPPER_NO_CONTRACT;

            elem.~WithinLoaderAllocatorData();
            elem._key = nullptr;
        }
        static const bool s_DestructPerEntryCleanupAction = true;

        static key_t GetKey(const WithinLoaderAllocatorData<TKey, TValue> &e)
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

        static const WithinLoaderAllocatorData<TKey, TValue> Null() { LIMITED_METHOD_CONTRACT; WithinLoaderAllocatorData<TKey, TValue> nullValue = {0}; return nullValue; }
        static bool IsNull(const WithinLoaderAllocatorData<TKey, TValue> &e) { LIMITED_METHOD_CONTRACT; return e._key == nullptr; }
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
#endif // !CROSSGEN_COMPILE

#ifndef CROSSGEN_COMPILE
    void EnsureManagedObjectsInitted();
    LAHASHDEPENDENTHASHTRACKERREF GetDependentTrackerForLoaderAllocator(LoaderAllocator* pLoaderAllocator);
    GCHEAPHASHOBJECTREF GetKeyToValueCrossLAHash(TKey key, LoaderAllocator* pValueLoaderAllocator);
    template <class Visitor>
    static bool VisitTracker(TKey key, LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe, Visitor &visitor);
#endif // !CROSSGEN_COMPILE

private:
    WithinLoaderAllocatorDataHash _withinLAHash;
    LoaderAllocator *_loaderAllocator = 0;
    LOADERHANDLE LAToDependentTrackerHash = 0;
    LOADERHANDLE KeyToDependentTrackersHash = 0;
};

#endif // CROSSLOADERALLOCATORHASH_H
