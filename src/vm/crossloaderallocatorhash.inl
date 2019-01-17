// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef CROSSLOADERALLOCATORHASH_INL
#define CROSSLOADERALLOCATORHASH_INL
#ifdef CROSSLOADERALLOCATORHASH_H

#ifndef CROSSGEN_COMPILE
#include "gcheaphashtable.inl"
#endif // !CROSSGEN_COMPILE

#ifndef DACCESS_COMPILE
template <class TKey_, class TValue_>
/*static*/ void NoRemoveDefaultCrossLoaderAllocatorHashTraits<TKey_, TValue_>::AddToValues(SArray<TValue> *pValues, const TValue& value)
{
    pValues->Append(value);
}

#ifndef CROSSGEN_COMPILE
template <class TKey_, class TValue_>
/*static*/ void NoRemoveDefaultCrossLoaderAllocatorHashTraits<TKey_, TValue_>::AddToValuesInHeapMemory(OBJECTREF &keyValueStore, OBJECTREF &newKeyValueStore, const TKey& key, const TValue& value)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    if (keyValueStore == NULL)
    {
        newKeyValueStore = AllocatePrimitiveArray(ELEMENT_TYPE_U, 3, FALSE);
        ((UPTRARRAYREF)newKeyValueStore)->GetDirectPointerToNonObjectElements()[0] = (UINT_PTR)key;
        ((UPTRARRAYREF)newKeyValueStore)->GetDirectPointerToNonObjectElements()[1] = (UINT_PTR)1;
        ((UPTRARRAYREF)newKeyValueStore)->GetDirectPointerToNonObjectElements()[2] = (UINT_PTR)value;
    }
    else
    {
        DWORD usedEntries = ((DWORD)((UPTRARRAYREF)keyValueStore)->GetDirectPointerToNonObjectElements()[1]) + 2;
        if (usedEntries <((UPTRARRAYREF)keyValueStore)->GetNumComponents())
        {
            // There isn't free space. Build a new, bigger array with the existing data 
            DWORD newSize = usedEntries * 2;
            if (newSize < usedEntries)
                COMPlusThrow(kOverflowException);

            newKeyValueStore = AllocatePrimitiveArray(ELEMENT_TYPE_U, newSize, FALSE);
            for (DWORD i = 0; i < usedEntries; i++)
            {
                ((UPTRARRAYREF)newKeyValueStore)->GetDirectPointerToNonObjectElements()[i] = ((UPTRARRAYREF)keyValueStore)->GetDirectPointerToNonObjectElements()[i];
            }

            keyValueStore = newKeyValueStore;
        }

        // There is free space. Append on the end
        ((UPTRARRAYREF)keyValueStore)->GetDirectPointerToNonObjectElements()[1] = usedEntries + 1;
        ((UPTRARRAYREF)keyValueStore)->GetDirectPointerToNonObjectElements()[usedEntries] = (UINT_PTR)value;
    }
}
#endif // !CROSSGEN_COMPILE
#endif //!DACCESS_COMPILE

#ifndef DACCESS_COMPILE
template <class TKey_, class TValue_>
/*static*/ void DefaultCrossLoaderAllocatorHashTraits<TKey_, TValue_>::DeleteValue(SArray<TValue> *pValues, const TValue& value)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    // TODO: Consider optimizing this by changing the add to ensure that the 
    // values list is sorted, and then doing a binary search for the value instead
    // of the linear search
    auto begin = pValues->Begin();
    auto end = pValues->End();

    for (auto i = begin; i != end; ++i)
    {
        if (*i == value)
        {
            pValues->Delete(i);
            return;
        }
    }

    return;
}

#ifndef CROSSGEN_COMPILE
template <class TKey_, class TValue_>
/*static*/ void DefaultCrossLoaderAllocatorHashTraits<TKey_, TValue_>::DeleteValueInHeapMemory(OBJECTREF keyValueStore, const TValue& value)
{
    LIMITED_METHOD_CONTRACT;

    // TODO: Consider optimizing this by changing the add to ensure that the 
    // values list is sorted, and then doing a binary search for the value instead
    // of the linear search

    UINT_PTR *dataInUptrArray = ((UPTRARRAYREF)keyValueStore)->GetDirectPointerToNonObjectElements();

    DWORD usedEntries = ((DWORD)dataInUptrArray[1]) + 2;

    DWORD foundEntry = 0xFFFFFFFF;
    UINT_PTR ptrToSearchFor = (UINT_PTR)value;

    for (DWORD iEntry = 2; iEntry < usedEntries; iEntry++)
    {
        if (dataInUptrArray[iEntry] == ptrToSearchFor)
        {
            memmove(dataInUptrArray + iEntry, dataInUptrArray + iEntry + 1, (usedEntries - iEntry - 1) * sizeof(UINT_PTR));
            dataInUptrArray[1] = usedEntries - 3;
            return;
        }
    }
}
#endif // !CROSSGEN_COMPILE
#endif //!DACCESS_COMPILE

#ifndef CROSSGEN_COMPILE
/*static*/ inline INT32 GCHeapHashDependentHashTrackerHashTraits::Hash(PtrTypeKey *pValue)
{
    LIMITED_METHOD_CONTRACT;
    return (INT32)*pValue;
}

/*static*/ inline INT32 GCHeapHashDependentHashTrackerHashTraits::Hash(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    LAHASHDEPENDENTHASHTRACKERREF value = (LAHASHDEPENDENTHASHTRACKERREF)GetValueAtIndex(pgcHeap, index);
    LoaderAllocator *pLoaderAllocator = value->GetLoaderAllocatorUnsafe();
    return Hash(&pLoaderAllocator);
}

/*static*/ inline bool GCHeapHashDependentHashTrackerHashTraits::DoesEntryMatchKey(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, PtrTypeKey *pKey)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    PTRARRAYREF arr((PTRARRAYREF)(*pgcHeap)->GetData());

    LAHASHDEPENDENTHASHTRACKERREF value = (LAHASHDEPENDENTHASHTRACKERREF)GetValueAtIndex(pgcHeap, index);

    return value->IsTrackerFor(*pKey);
}

/*static*/ inline bool GCHeapHashDependentHashTrackerHashTraits::IsDeleted(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    OBJECTREF valueInHeap = GetValueAtIndex(pgcHeap, index);

    if (valueInHeap == NULL)
        return false;

    if (*pgcHeap == valueInHeap)
        return true;

    // This is a tricky bit of logic used which detects freed loader allocators lazily
    // and deletes them from the GCHeapHash while looking up or otherwise walking the hashtable
    // for any purpose.
    LAHASHDEPENDENTHASHTRACKERREF value = (LAHASHDEPENDENTHASHTRACKERREF)valueInHeap;
    if (!value->IsLoaderAllocatorLive())
    {
#ifndef DACCESS_COMPILE
        PTRARRAYREF arr((PTRARRAYREF)(*pgcHeap)->GetData());

        if (arr == NULL)
            COMPlusThrow(kNullReferenceException);

        if ((INT32)arr->GetNumComponents() < index)
            COMPlusThrow(kIndexOutOfRangeException);

        arr->SetAt(index, *pgcHeap);
        (*pgcHeap)->DecrementCount();
#endif // DACCESS_COMPILE

        return true;
    }

    return false;
}

template <class TKey>
/*static*/ INT32 GCHeapHashKeyToDependentTrackersHashTraits::Hash(TKey *pValue)
{
    LIMITED_METHOD_CONTRACT;
    return (INT32)*pValue;
}

/*static*/ inline INT32 GCHeapHashKeyToDependentTrackersHashTraits::Hash(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    LAHASHDEPENDENTHASHTRACKERREF value = (LAHASHDEPENDENTHASHTRACKERREF)GetValueAtIndex(pgcHeap, index);

    void * key = value->GetLoaderAllocatorUnsafe();
    return Hash(&key);
}

template<class TKey>
/*static*/ bool GCHeapHashKeyToDependentTrackersHashTraits::DoesEntryMatchKey(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, TKey *pKey)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    PTRARRAYREF arr((PTRARRAYREF)(*pgcHeap)->GetData());

    LAHASHDEPENDENTHASHTRACKERREF value = (LAHASHDEPENDENTHASHTRACKERREF)GetValueAtIndex(pgcHeap, index);

    if ((INT_PTR)value->GetLoaderAllocatorUnsafe() != (INT_PTR)*pKey)
        return false;

    // At this point we know the LA heap values match, but we need to check the dependent handle to 
    // ensure that the LA is actually still alive. In theory, we could have freed the LA, and allocated
    // a new one with the same pointer value.

    return value->IsLoaderAllocatorLive();
}
#endif // !CROSSGEN_COMPILE

#ifndef DACCESS_COMPILE
template <class TRAITS>
void CrossLoaderAllocatorHash<TRAITS>::Add(TKey key, TValue value, LoaderAllocator *pLoaderAllocatorOfValue)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    // This data structure actually doesn't have this invariant, but it is expected that uses of this
    // data structure will require that the key's loader allocator is the same as that of this data structure.
    _ASSERTE(key->GetLoaderAllocator() == _loaderAllocator);

    // Get pointer to key data in SHash used for non-LA crossing data.

    const WithinLoaderAllocatorDataStruct* pAssociatedWithKeyData = (WithinLoaderAllocatorDataStruct*)_withinLAHash.LookupPtr(key);
    if (pAssociatedWithKeyData == NULL)
    {
        // If non-LA crossing data structure doesn't exist, add it, and then capture current pointer
        WithinLoaderAllocatorDataStruct newValue;
        newValue._key = key;
        _withinLAHash.Add(newValue);
        pAssociatedWithKeyData = _withinLAHash.LookupPtr(key);
    }

    // Check to see if value can be added to this data structure directly.
#ifndef CROSSGEN_COMPILE
    if (_loaderAllocator == pLoaderAllocatorOfValue)
#endif //!CROSSGEN_COMPILE
    {
        TRAITS::AddToValues(&pAssociatedWithKeyData->_values, value);
    }
#ifndef CROSSGEN_COMPILE
    else
    {
        // Must mark the assoacitedWithKeyData with the flag that indicates there are non-local entries
        pAssociatedWithKeyData->_key = (TKey)((INT_PTR)pAssociatedWithKeyData->_key | 1);

        // Must add it to the cross LA structure
        GCHEAPHASHOBJECTREF gcheapKeyToValue = GetKeyToValueCrossLAHash(key, pLoaderAllocatorOfValue);

        struct {
            KeyToValuesGCHeapHash keyToValuePerLAHash;
            OBJECTREF keyValueStore;
            OBJECTREF newKeyValueStore;
            OBJECTREF objRefNull;
        } gc;

        ZeroMemory(&gc, sizeof(gc));

        gc.keyToValuePerLAHash = KeyToValuesGCHeapHash(gcheapKeyToValue);

        GCPROTECT_BEGIN(gc)
        {
            INT32 indexInKeyValueHash = gc.keyToValuePerLAHash.GetValueIndex(&key);
            if (indexInKeyValueHash != -1)
            {
                gc.keyToValuePerLAHash.GetElement(indexInKeyValueHash, gc.keyValueStore);

                TRAITS::AddToValuesInHeapMemory(gc.keyValueStore, gc.newKeyValueStore, key, value);

                if (gc.newKeyValueStore != NULL)
                {
                    gc.keyToValuePerLAHash.SetElement(indexInKeyValueHash, gc.newKeyValueStore);
                }
            }
            else
            {
                TRAITS::AddToValuesInHeapMemory(gc.objRefNull, gc.newKeyValueStore, key, value);

                gc.keyToValuePerLAHash.Add(&key, [&gc](PTRARRAYREF arr, INT32 index)
                {
                    arr->SetAt(index, gc.keyValueStore);
                });
            }
        }
        GCPROTECT_END();
    }
#endif // !CROSSGEN_COMPILE
}
#endif // !DACCESS_COMPILE

#ifndef DACCESS_COMPILE
template <class TRAITS>
void CrossLoaderAllocatorHash<TRAITS>::Remove(TKey key, TValue value, LoaderAllocator *pLoaderAllocatorOfValue)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    // This data structure actually doesn't have this invariant, but it is expected that uses of this
    // data structure will require that the key's loader allocator is the same as that of this data structure.
    _ASSERTE(key->GetLoaderAllocator() == _loaderAllocator);

    // Get pointer to key data in SHash used for non-LA crossing data.

    const WithinLoaderAllocatorDataStruct* pAssociatedWithKeyData = (WithinLoaderAllocatorDataStruct*)_withinLAHash.LookupPtr(key);
    if (pAssociatedWithKeyData == NULL)
    {
        // If non-LA crossing data structure doesn't exist, then the value isn't present. Simply return.
        return;
    }

    // Check to see if value can be added to this data structure directly.
#ifndef CROSSGEN_COMPILE
    if (_loaderAllocator == pLoaderAllocatorOfValue)
#endif //!CROSSGEN_COMPILE
    {
        TRAITS::DeleteValue(&pAssociatedWithKeyData->_values, value);
    }
#ifndef CROSSGEN_COMPILE
    else
    {
        // Must remove it from the cross LA structure
        GCHEAPHASHOBJECTREF gcheapKeyToValue = GetKeyToValueCrossLAHash(key, pLoaderAllocatorOfValue);

        struct {
            KeyToValuesGCHeapHash keyToValuePerLAHash;
            OBJECTREF keyValueStore;
        } gc;

        ZeroMemory(&gc, sizeof(gc));

        gc.keyToValuePerLAHash = KeyToValuesGCHeapHash(gcheapKeyToValue);

        GCPROTECT_BEGIN(gc)
        {
            INT32 indexInKeyValueHash = gc.keyToValuePerLAHash.GetValueIndex(&key);
            if (indexInKeyValueHash != -1)
            {
                gc.keyToValuePerLAHash.GetElement(indexInKeyValueHash, gc.keyValueStore);
                TRAITS::DeleteValueInHeapMemory(gc.keyValueStore, value);
            }
        }
        GCPROTECT_END();
    }
#endif // !CROSSGEN_COMPILE
}
#endif // !DACCESS_COMPILE

template <class TRAITS>
template <class Visitor>
bool CrossLoaderAllocatorHash<TRAITS>::VisitValuesOfKey(TKey key, Visitor &visitor)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    // This data structure actually doesn't have this invariant, but it is expected that uses of this
    // data structure will require that the key's loader allocator is the same as that of this data structure.
    _ASSERTE(key->GetLoaderAllocator() == _loaderAllocator);

    // Get pointer to key data in SHash used for non-LA crossing data.

    const WithinLoaderAllocatorDataStruct* pAssociatedWithKeyData = _withinLAHash.LookupPtr(key);
    if (pAssociatedWithKeyData == NULL)
        return true;

    // Visit data local to local LA
    auto beginLocalWalk = pAssociatedWithKeyData->_values.Begin();
    auto endLocalWalk = pAssociatedWithKeyData->_values.Begin();
    OBJECTREF nullObjectRef = NULL;
    for (auto i = beginLocalWalk; i != endLocalWalk; ++i)
    {
        if (!visitor(nullObjectRef, *i))
            return false;
    }

    bool result = true;
#ifndef CROSSGEN_COMPILE
    if (((INT_PTR)pAssociatedWithKeyData->_key & 1) == 1)
    {
        // There is non-local data;
        struct 
        {
            GCHeapHashKeyToDependentTrackersHash keyToTrackersHash;
            GCHeapHashDependentHashTrackerHash dependentTrackerHash;
            LAHASHDEPENDENTHASHTRACKERREF dependentTrackerMaybe;
            LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
            LAHASHKEYTOTRACKERSREF hashKeyToTrackers;
            GCHEAPHASHOBJECTREF returnValue;
        } gc;
        ZeroMemory(&gc, sizeof(gc));
        GCPROTECT_BEGIN(gc)
        {
            gc.keyToTrackersHash = GCHeapHashKeyToDependentTrackersHash((GCHEAPHASHOBJECTREF)_loaderAllocator->GetHandleValue(KeyToDependentTrackersHash));
            INT32 index = gc.keyToTrackersHash.GetValueIndex(&key);
            if (index != -1)
            {
                // We have an entry in the hashtable for the key/dependenthandle.
                gc.keyToTrackersHash.GetElement(index, gc.hashKeyToTrackers);

                // Now gc.hashKeyToTrackers is filled in.

                // Is there a single dependenttracker here, or a set.

                if (gc.hashKeyToTrackers->_trackerOrTrackerSet->GetMethodTable() == MscorlibBinder::GetClass(CLASS__LAHASHDEPENDENTHASHTRACKER))
                {
                    gc.dependentTracker = (LAHASHDEPENDENTHASHTRACKERREF)gc.hashKeyToTrackers->_trackerOrTrackerSet;
                    result = VisitTracker(key, gc.dependentTracker, visitor);
                }
                else
                {
                    gc.dependentTrackerHash = GCHeapHashDependentHashTrackerHash(gc.hashKeyToTrackers->_trackerOrTrackerSet);
                    result = gc.dependentTrackerHash.VisitAllEntryIndices(VisitIndividualEntryKeyValueHash<Visitor>(key, &visitor, &gc.dependentTrackerHash));
                }
            }
        }
        GCPROTECT_END();
    }
#endif // !CROSSGEN_COMPILE

    return result;
}

template <class TRAITS>
template <class Visitor>
bool CrossLoaderAllocatorHash<TRAITS>::VisitAllKeyValuePairs(Visitor &visitor)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    // This data structure actually doesn't have this invariant, but it is expected that uses of this
    // data structure will require that the key's loader allocator is the same as that of this data structure.
    _ASSERTE(key->GetLoaderAllocator() == _loaderAllocator);

    // Get pointer to key data in SHash used for non-LA crossing data.

    auto beginLAHash = _withinLAHash.Begin();
    auto endLAHash = _withinLAHash.End();

    // Walk all entries in LAHash
    for (auto iLAHash = beginLAHash; iLAHash != endLAHash; ++iLAHash)
    {
        auto beginLocalWalk = pAssociatedWithKeyData->_values.Begin();
        auto endLocalWalk = pAssociatedWithKeyData->_values.Begin();
        OBJECTREF nullObjectRef = NULL;
        for (auto i = beginLocalWalk; i != endLocalWalk; ++i)
        {
            if (!visitor(nullObjectRef, pAssociatedWithKeyData->_key, *i))
                return false;
        }
    }

    bool result = true;
#ifndef CROSSGEN_COMPILE
    if (LAToDependentTrackerHash != NULL)
    {
        // If there are remote entries, walk them

        // There is non-local data;
        struct 
        {
            GCHeapHashKeyToDependentTrackersHash keyToTrackersHash;
            GCHeapHashDependentHashTrackerHash dependentTrackerHash;
            LAHASHDEPENDENTHASHTRACKERREF dependentTrackerMaybe;
            LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
            LAHASHKEYTOTRACKERSREF hashKeyToTrackers;
            GCHEAPHASHOBJECTREF returnValue;
        } gc;
        ZeroMemory(&gc, sizeof(gc));
        GCPROTECT_BEGIN(gc)
        {
            gc.dependentTrackerHash = GCHeapHashDependentHashTrackerHash((GCHEAPHASHOBJECTREF)_loaderAllocator->GetHandleValue(KeyToDependentTrackersHash));
            result = gc.dependentTrackerHash.VisitAllEntryIndices(VisitAllEntryDependentTrackerHash<Visitor>(&visitor, &gc.dependentTrackerHash));
        }
        GCPROTECT_END();
    }
#endif // !CROSSGEN_COMPILE

    return result;
}

#ifndef DACCESS_COMPILE
template <class TRAITS>
void CrossLoaderAllocatorHash<TRAITS>::RemoveAll(TKey key)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    // This data structure actually doesn't have this invariant, but it is expected that uses of this
    // data structure will require that the key's loader allocator is the same as that of this data structure.
    _ASSERTE(key->GetLoaderAllocator() == _loaderAllocator);

    // Get pointer to key data in SHash used for non-LA crossing data.

    const WithinLoaderAllocatorDataStruct* pAssociatedWithKeyData = _withinLAHash.LookupPtr(key);
    if (pAssociatedWithKeyData == NULL)
        return; // There are no entries associated with key

#ifndef CROSSGEN_COMPILE
    if (((INT_PTR)pAssociatedWithKeyData->_key & 1) == 1)
    {
        // There is non-local data;
        struct 
        {
            GCHeapHashKeyToDependentTrackersHash keyToTrackersHash;
            GCHeapHashDependentHashTrackerHash dependentTrackerHash;
            LAHASHDEPENDENTHASHTRACKERREF dependentTrackerMaybe;
            LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
            LAHASHKEYTOTRACKERSREF hashKeyToTrackers;
            GCHEAPHASHOBJECTREF returnValue;
        } gc;
        ZeroMemory(&gc, sizeof(gc));
        GCPROTECT_BEGIN(gc)
        {
            gc.keyToTrackersHash = GCHeapHashKeyToDependentTrackersHash((GCHEAPHASHOBJECTREF)_loaderAllocator->GetHandleValue(KeyToDependentTrackersHash));
            INT32 index = gc.keyToTrackersHash.GetValueIndex(&key);
            if (index != -1)
            {
                // We have an entry in the hashtable for the key/dependenthandle.
                gc.keyToTrackersHash.GetElement(index, gc.hashKeyToTrackers);

                // Now gc.hashKeyToTrackers is filled in.

                // Is there a single dependenttracker here, or a set.

                if (gc.hashKeyToTrackers->_trackerOrTrackerSet->GetMethodTable() == MscorlibBinder::GetClass(CLASS__LAHASHDEPENDENTHASHTRACKER))
                {
                    gc.dependentTracker = (LAHASHDEPENDENTHASHTRACKERREF)gc.hashKeyToTrackers->_trackerOrTrackerSet;
                    result = DeleteEntryTracker(key, gc.dependentTracker);
                }
                else
                {
                    gc.dependentTrackerHash = GCHeapHashDependentHashTrackerHash(gc.hashKeyToTrackers->_trackerOrTrackerSet);
                    result = gc.dependentTrackerHash.VisitAllEntryIndices(DeleteIndividualEntryKeyValueHash<Visitor>(key, &gc.dependentTrackerHash));
                }

                // Remove entry from key to tracker hash
                gc.keyToTrackersHash.DeleteEntry(&key);
            }
        }
        GCPROTECT_END();
    }
#endif // !CROSSGEN_COMPILE

    _withinLAHash.Remove(key);
}
#endif // !DACCESS_COMPILE

template <class TRAITS>
void CrossLoaderAllocatorHash<TRAITS>::Init(LoaderAllocator *pAssociatedLoaderAllocator)
{
    _loaderAllocator = pAssociatedLoaderAllocator;
}

#ifndef CROSSGEN_COMPILE
template <class TRAITS>
template <class Visitor>
/*static*/ bool CrossLoaderAllocatorHash<TRAITS>::VisitTracker(TKey key, LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe, Visitor &visitor)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    struct 
    {
        LAHASHDEPENDENTHASHTRACKERREF tracker;
        OBJECTREF loaderAllocatorRef;
        GCHEAPHASHOBJECTREF keyToValuesHashObject;
        KeyToValuesGCHeapHash keyToValuesHash;
        UPTRARRAYREF keyValueStore;
    }gc;

    ZeroMemory(&gc, sizeof(gc));
    gc.tracker = trackerUnsafe;

    bool result = true;

    GCPROTECT_BEGIN(gc);
    {
        gc.tracker->GetDependentAndLoaderAllocator(&gc.loaderAllocatorRef, &gc.keyToValuesHashObject);
        if (gc.keyToValuesHashObject != NULL)
        {
            gc.keyToValuesHash = KeyToValuesGCHeapHash(gc.keyToValuesHashObject);
            INT32 indexInKeyValueHash = gc.keyToValuesHash.GetValueIndex(&key);
            if (indexInKeyValueHash != -1)
            {
                gc.keyToValuesHash.GetElement(indexInKeyValueHash, gc.keyValueStore);

                _ASSERTE(gc.keyValueStore->GetDirectPointerToNonObjectElements()[0] == (UINT_PTR)key);

                // We now have the keyValueStore. Check to see if there is free space within it
                DWORD usedEntries = ((DWORD)gc.keyValueStore->GetDirectPointerToNonObjectElements()[1]) + 2;
                for (DWORD index = 2; index < usedEntries; ++index)
                {
                    result = visitor(gc.loaderAllocatorRef, (TKey)gc.keyValueStore->GetDirectPointerToNonObjectElements()[index]);
                    if (!result)
                    {
                        goto readyToReturn; // Early out
                    }
                }
            }
        }
readyToReturn:;
    }
    GCPROTECT_END();

    return result;
}

template <class TRAITS>
template <class Visitor>
/*static*/ bool CrossLoaderAllocatorHash<TRAITS>::VisitTrackerAllEntries(LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe, Visitor &visitor)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    struct _gcStruct
    {
        LAHASHDEPENDENTHASHTRACKERREF tracker;
        OBJECTREF loaderAllocatorRef;
        GCHEAPHASHOBJECTREF keyToValuesHashObject;
        KeyToValuesGCHeapHash keyToValuesHash;
        UPTRARRAYREF keyValueStore;
    }gc;

    class VisitAllEntryKeyValueHash
    {
        public:
        Visitor *_pVisitor;
        KeyToValuesGCHeapHash *_pKeysToValueHash;
        UPTRARRAYREF *_pKeyValueStore;
        OBJECTREF *pLoaderAllocatorRef;

        VisitAllEntryKeyValueHash(Visitor *pVisitor,  KeyToValuesGCHeapHash *pKeysToValueHash, UPTRARRAYREF *pKeyValueStore, OBJECTREF *pLoaderAllocatorRef) : 
            _pVisitor(pVisitor),
            _pKeysToValueHash(pKeysToValueHash)
            _pKeyValueStore(pKeyValueStore),
            _pLoaderAllocatorRef(pLoaderAllocatorRef)
            {}

        bool operator()(INT32 index)
        {
            WRAPPER_NO_CONTRACT;

            _pKeysToValueHash->GetElement(index, *_pKeyValueStore);

            DWORD usedEntries = ((DWORD)(*_pKeyValueStore)->GetDirectPointerToNonObjectElements()[1]) + 2;
            for (DWORD index = 2; index < usedEntries; ++index)
            {
                result = visitor(*pLoaderAllocatorRef, (TKey)(*_pKeyValueStore)->GetDirectPointerToNonObjectElements()[0], (TValue)(*_pKeyValueStore)->GetDirectPointerToNonObjectElements()[index]);
                if (!result)
                {
                    return false;
                }
            }
            return true;
        }
    };

    ZeroMemory(&gc, sizeof(gc));
    gc.tracker = trackerUnsafe;

    bool result = true;

    GCPROTECT_BEGIN(gc);
    {
        gc.tracker->GetDependentAndLoaderAllocator(&gc.loaderAllocatorRef, &gc.keyToValuesHashObject);
        if (gc.keyToValuesHashObject != NULL)
        {
            gc.keyToValuesHash = KeyToValuesGCHeapHash(gc.keyToValuesHashObject);
            result = gc.keyToValuesHash.VisitAllEntryIndices(VisitAllEntryKeyValueHash<Visitor>(&visitor, &gc.keyToValuesHash, &gc.keyValueStore, &gc.loaderAllocatorRef));
        }
    }
    GCPROTECT_END();

    return result;
}

template <class TRAITS>
/*static*/ void CrossLoaderAllocatorHash<TRAITS>::DeleteEntryTracker(TKey key, LAHASHDEPENDENTHASHTRACKERREF trackerUnsafe)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    struct 
    {
        LAHASHDEPENDENTHASHTRACKERREF tracker;
        OBJECTREF loaderAllocatorRef;
        GCHEAPHASHOBJECTREF keyToValuesHashObject;
        KeyToValuesGCHeapHash keyToValuesHash;
        UPTRARRAYREF keyValueStore;
    }gc;

    ZeroMemory(&gc, sizeof(gc));
    gc.tracker = trackerUnsafe;

    GCPROTECT_BEGIN(gc);
    {
        gc.tracker->GetDependentAndLoaderAllocator(&gc.loaderAllocatorRef, &gc.keyToValuesHashObject);
        if (gc.keyToValuesHashObject != NULL)
        {
            gc.keyToValuesHash = KeyToValuesGCHeapHash(gc.keyToValuesHashObject);
            gc.keyToValuesHash.DeleteEntry(&key);
        }
    }
    GCPROTECT_END();
}
#endif // !CROSSGEN_COMPILE

#ifndef CROSSGEN_COMPILE
template <class TRAITS>
void CrossLoaderAllocatorHash<TRAITS>::EnsureManagedObjectsInitted()
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    if (LAToDependentTrackerHash == NULL)
    {
        OBJECTREF laToDependentHandleHash = AllocateObject(MscorlibBinder::GetClass(CLASS__GCHEAPHASH));
        LAToDependentTrackerHash = _loaderAllocator->AllocateHandle(laToDependentHandleHash);
    }

    if (KeyToDependentTrackersHash == NULL)
    {
        OBJECTREF keyToDependentTrackersHash = AllocateObject(MscorlibBinder::GetClass(CLASS__GCHEAPHASH));
        KeyToDependentTrackersHash = _loaderAllocator->AllocateHandle(keyToDependentTrackersHash);
    }
}
#endif // !CROSSGEN_COMPILE

#ifndef CROSSGEN_COMPILE
template <class TRAITS>
LAHASHDEPENDENTHASHTRACKERREF CrossLoaderAllocatorHash<TRAITS>::GetDependentTrackerForLoaderAllocator(LoaderAllocator* pLoaderAllocator)
{
    struct 
    {
        GCHeapHashDependentHashTrackerHash dependentTrackerHash;
        LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
        GCHEAPHASHOBJECTREF GCHeapHashForKeyToValueStore;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    GCPROTECT_BEGIN(gc)
    {
        gc.dependentTrackerHash = GCHeapHashDependentHashTrackerHash((GCHEAPHASHOBJECTREF)_loaderAllocator->GetHandleValue(LAToDependentTrackerHash));
        INT32 index = gc.dependentTrackerHash.GetValueIndex(&pLoaderAllocator);
        if (index != -1)
        {
            // We have an entry in the hashtable for the key/dependenthandle.
            gc.dependentTrackerHash.GetElement(index, gc.dependentTracker);
        }
        else
        {
            gc.dependentTracker = (LAHASHDEPENDENTHASHTRACKERREF)AllocateObject(MscorlibBinder::GetClass(CLASS__LAHASHDEPENDENTHASHTRACKER));
            gc.GCHeapHashForKeyToValueStore = (GCHEAPHASHOBJECTREF)AllocateObject(MscorlibBinder::GetClass(CLASS__GCHEAPHASH));
            OBJECTHANDLE dependentHandle = GetAppDomain()->CreateDependentHandle(pLoaderAllocator->GetExposedObject(), gc.GCHeapHashForKeyToValueStore);
            gc.dependentTracker->Init(dependentHandle, pLoaderAllocator);
            gc.dependentTrackerHash.Add(&pLoaderAllocator, [&gc](PTRARRAYREF arr, INT32 index)
            {
                arr->SetAt(index, (OBJECTREF)gc.dependentTracker);
            });
        }
    }
    GCPROTECT_END();

    return gc.dependentTracker;
}
#endif // !CROSSGEN_COMPILE

#ifndef CROSSGEN_COMPILE
template <class TRAITS>
GCHEAPHASHOBJECTREF CrossLoaderAllocatorHash<TRAITS>::GetKeyToValueCrossLAHash(TKey key, LoaderAllocator* pValueLoaderAllocator)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    EnsureManagedObjectsInitted();
    struct 
    {
        GCHeapHashKeyToDependentTrackersHash keyToTrackersHash;
        GCHeapHashDependentHashTrackerHash dependentTrackerHash;
        LAHASHDEPENDENTHASHTRACKERREF dependentTrackerMaybe;
        LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
        LAHASHKEYTOTRACKERSREF hashKeyToTrackers;
        GCHEAPHASHOBJECTREF returnValue;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    GCPROTECT_BEGIN(gc)
    {
        gc.keyToTrackersHash = GCHeapHashKeyToDependentTrackersHash((GCHEAPHASHOBJECTREF)_loaderAllocator->GetHandleValue(KeyToDependentTrackersHash));
        INT32 index = gc.keyToTrackersHash.GetValueIndex(&key);
        if (index == -1)
        {
            gc.hashKeyToTrackers = (LAHASHKEYTOTRACKERSREF)AllocateObject(MscorlibBinder::GetClass(CLASS__LAHASHKEYTOTRACKERS));
            gc.hashKeyToTrackers->_key = (INT_PTR)key;
            gc.dependentTracker = GetDependentTrackerForLoaderAllocator(pValueLoaderAllocator);
            gc.hashKeyToTrackers->_trackerOrTrackerSet = gc.dependentTracker;
            gc.keyToTrackersHash.Add(&key, [&gc](PTRARRAYREF arr, INT32 index)
                {
                    arr->SetAt(index, (OBJECTREF)gc.hashKeyToTrackers);
                });
        }
        else
        {

            // We already have an entry in the hashtable for the key/dependenthandle. Get the right dependent tracker...
            gc.keyToTrackersHash.GetElement(index, gc.hashKeyToTrackers);

            // Now gc.hashKeyToTrackers is filled in.

            // Is there a single dependenttracker here, or a set.
            if (gc.hashKeyToTrackers->_trackerOrTrackerSet->GetMethodTable() == MscorlibBinder::GetClass(CLASS__LAHASHDEPENDENTHASHTRACKER))
            {
                gc.dependentTrackerMaybe = (LAHASHDEPENDENTHASHTRACKERREF)gc.hashKeyToTrackers->_trackerOrTrackerSet;
                if (gc.dependentTrackerMaybe->IsTrackerFor(pValueLoaderAllocator))
                {
                    // We've found the right dependent tracker.
                    gc.dependentTracker = gc.dependentTrackerMaybe;
                }
                else
                {
                    gc.dependentTracker = GetDependentTrackerForLoaderAllocator(pValueLoaderAllocator);
                    if (!gc.dependentTrackerMaybe->IsLoaderAllocatorLive())
                    {
                        SetObjectReference(&gc.hashKeyToTrackers->_trackerOrTrackerSet, gc.dependentTracker, GetAppDomain());
                    }
                    else
                    {
                        // Allocate the dependent tracker hash
                        // Fill with the existing dependentTrackerMaybe, and gc.DependentTracker
                        gc.dependentTrackerHash = GCHeapHashDependentHashTrackerHash(AllocateObject(MscorlibBinder::GetClass(CLASS__GCHEAPHASH)));
                        LoaderAllocator *pLoaderAllocatorKey = gc.dependentTracker->GetLoaderAllocatorUnsafe();
                        gc.dependentTrackerHash.Add(&pLoaderAllocatorKey, [&gc](PTRARRAYREF arr, INT32 index)
                            {
                                arr->SetAt(index, (OBJECTREF)gc.dependentTracker);
                            });
                        pLoaderAllocatorKey = gc.dependentTrackerMaybe->GetLoaderAllocatorUnsafe();
                        gc.dependentTrackerHash.Add(&pLoaderAllocatorKey, [&gc](PTRARRAYREF arr, INT32 index)
                            {
                                arr->SetAt(index, (OBJECTREF)gc.dependentTrackerMaybe);
                            });
                    }
                }
            }
            else
            {
                gc.dependentTrackerHash = GCHeapHashDependentHashTrackerHash(gc.hashKeyToTrackers->_trackerOrTrackerSet);

                INT32 indexOfTracker = gc.dependentTrackerHash.GetValueIndex(&pValueLoaderAllocator);
                if (indexOfTracker == -1)
                {
                    // Dependent tracker not yet attached to this key
                    
                    // Get dependent tracker
                    gc.dependentTracker = GetDependentTrackerForLoaderAllocator(pValueLoaderAllocator);
                    gc.dependentTrackerHash.Add(&pValueLoaderAllocator, [&gc](PTRARRAYREF arr, INT32 index)
                        {
                            arr->SetAt(index, (OBJECTREF)gc.dependentTracker);
                        });
                }
                else
                {
                    gc.dependentTrackerHash.GetElement(indexOfTracker, gc.dependentTracker);
                }
            }
        }

        // At this stage gc.dependentTracker is setup to have a good value
        gc.returnValue = gc.dependentTracker->GetDependentTarget();
    }
    GCPROTECT_END();

    return gc.returnValue;
}
#endif // !CROSSGEN_COMPILE

#endif // CROSSLOADERALLOCATORHASH_H
#endif // CROSSLOADERALLOCATORHASH_INL
