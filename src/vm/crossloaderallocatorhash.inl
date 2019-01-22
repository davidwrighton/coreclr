// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef CROSSLOADERALLOCATORHASH_INL
#define CROSSLOADERALLOCATORHASH_INL
#ifdef CROSSLOADERALLOCATORHASH_H
#ifndef CROSSGEN_COMPILE

#include "gcheaphashtable.inl"

#ifndef DACCESS_COMPILE
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
        newKeyValueStore = AllocatePrimitiveArray(ELEMENT_TYPE_U, (value == NULL) ? 2 : 3, FALSE);
        ((UPTRARRAYREF)newKeyValueStore)->GetDirectPointerToNonObjectElements()[0] = (UINT_PTR)key;
        ((UPTRARRAYREF)newKeyValueStore)->GetDirectPointerToNonObjectElements()[1] = (UINT_PTR)1;
        if (value != NULL)
        {
            ((UPTRARRAYREF)newKeyValueStore)->GetDirectPointerToNonObjectElements()[2] = (UINT_PTR)value;
        }
    }
    else if (value != NULL)
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
#endif //!DACCESS_COMPILE

#ifndef DACCESS_COMPILE
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
#endif //!DACCESS_COMPILE

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

    EnsureManagedObjectsInitted();

    struct {
        GCHeapHashKeyToDependentTrackersHash keyToTrackersHash;
        KeyToValuesGCHeapHash keyToValuePerLAHash;
        OBJECTREF keyValueStore;
        OBJECTREF newKeyValueStore;
        OBJECTREF objRefNull;
        LAHASHKEYTOTRACKERSREF hashKeyToTrackers;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    GCPROTECT_BEGIN(gc)
    {
        bool addToKeyValuesHash = false;
        // This data structure actually doesn't have this invariant, but it is expected that uses of this
        // data structure will require that the key's loader allocator is the same as that of this data structure.
        _ASSERTE(key->GetLoaderAllocator() == _loaderAllocator);

        gc.keyToTrackersHash = GCHeapHashKeyToDependentTrackersHash((GCHEAPHASHOBJECTREF)ObjectFromHandle(KeyToDependentTrackersHash));
        INT32 index = gc.keyToTrackersHash.GetValueIndex(&key);

        if (index == -1)
        {
            gc.hashKeyToTrackers = (LAHASHKEYTOTRACKERSREF)AllocateObject(MscorlibBinder::GetClass(CLASS__LAHASHKEYTOTRACKERS));
            addToKeyValuesHash = true;
            TRAITS::AddToValuesInHeapMemory(gc.keyValueStore, gc.newKeyValueStore, key, pLoaderAllocatorOfValue == _loaderAllocator ? value : NULL);
            SetObjectReference(&gc.hashKeyToTrackers->_laLocalKeyValueStore, gc.newKeyValueStore, GetAppDomain());
        }
        else
        {
            gc.keyToTrackersHash.GetElement(index, gc.hashKeyToTrackers);
            gc.keyValueStore = gc.hashKeyToTrackers->_laLocalKeyValueStore;
        }

        if ((pLoaderAllocatorOfValue == _loaderAllocator) && (gc.newKeyValueStore == NULL))
        {
            TRAITS::AddToValuesInHeapMemory(gc.keyValueStore, gc.newKeyValueStore, key, pLoaderAllocatorOfValue == _loaderAllocator ? value : NULL);
        }

        if (gc.newKeyValueStore != NULL)
        {
            SetObjectReference(&gc.hashKeyToTrackers->_laLocalKeyValueStore, gc.newKeyValueStore, GetAppDomain());
        }

        if (addToKeyValuesHash)
        {
            gc.keyToTrackersHash.Add(&key, [&gc](PTRARRAYREF arr, INT32 index)
            {
                arr->SetAt(index, (OBJECTREF)gc.hashKeyToTrackers);
            });
        }

        // If the LoaderAllocator matches, we've finished adding by now, otherwise, we need to get the remove hash and work with that
        if (pLoaderAllocatorOfValue != _loaderAllocator)
        {
            // Must add it to the cross LA structure
            GCHEAPHASHOBJECTREF gcheapKeyToValue = GetKeyToValueCrossLAHashForHashkeyToTrackers(gc.hashKeyToTrackers, pLoaderAllocatorOfValue);

            gc.keyToValuePerLAHash = KeyToValuesGCHeapHash(gcheapKeyToValue);

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
    }
    GCPROTECT_END();
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

    if (KeyToDependentTrackersHash == NULL)
    {
        // If the heap objects haven't been initted, then there is nothing to delete
        return;
    }

    struct {
        GCHeapHashKeyToDependentTrackersHash keyToTrackersHash;
        KeyToValuesGCHeapHash keyToValuePerLAHash;
        LAHASHKEYTOTRACKERSREF hashKeyToTrackers;
        OBJECTREF keyValueStore;
    } gc;

    ZeroMemory(&gc, sizeof(gc));
    GCPROTECT_BEGIN(gc)
    {
        gc.keyToTrackersHash = GCHeapHashKeyToDependentTrackersHash((GCHEAPHASHOBJECTREF)ObjectFromHandle(KeyToDependentTrackersHash));
        INT32 index = gc.keyToTrackersHash.GetValueIndex(&key);

        if (index != -1)
        {
            gc.keyToTrackersHash.GetElement(index, gc.hashKeyToTrackers);
            gc.keyValueStore = gc.hashKeyToTrackers->_laLocalKeyValueStore;

            // Check to see if value can be added to this data structure directly.
            if (_loaderAllocator == pLoaderAllocatorOfValue)
            {
                TRAITS::DeleteValueInHeapMemory(gc.keyValueStore, value);
            }
            else
            {
                // Must remove it from the cross LA structure
                GCHEAPHASHOBJECTREF gcheapKeyToValue = GetKeyToValueCrossLAHashForHashkeyToTrackers(gc.hashKeyToTrackers, pLoaderAllocatorOfValue);

                gc.keyToValuePerLAHash = KeyToValuesGCHeapHash(gcheapKeyToValue);

                INT32 indexInKeyValueHash = gc.keyToValuePerLAHash.GetValueIndex(&key);
                if (indexInKeyValueHash != -1)
                {
                    gc.keyToValuePerLAHash.GetElement(indexInKeyValueHash, gc.keyValueStore);
                    TRAITS::DeleteValueInHeapMemory(gc.keyValueStore, value);
                }
            }
        }
    }
    GCPROTECT_END();
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

    // Check to see that something has been added
    if (KeyToDependentTrackersHash == NULL)
        return true;

    bool result = true;
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
        gc.keyToTrackersHash = GCHeapHashKeyToDependentTrackersHash((GCHEAPHASHOBJECTREF)ObjectFromHandle(KeyToDependentTrackersHash));
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
        bool result = true;
        if (KeyToDependentTrackersHash != NULL)
        {
            // Visit all local entries
            gc.keyToTrackersHash = GCHeapHashKeyToDependentTrackersHash((GCHEAPHASHOBJECTREF)ObjectFromHandle(KeyToDependentTrackersHash));
            result = gc.keyToTrackersHash.VisitAllEntryIndices(VisitAllEntryKeyToDependentTrackerHash<Visitor>(&visitor, &gc.keyToTrackersHash));
        }

        if (LAToDependentTrackerHash != NULL)
        {
            // Visit the non-local data
            gc.dependentTrackerHash = GCHeapHashDependentHashTrackerHash((GCHEAPHASHOBJECTREF)ObjectFromHandle(LAToDependentTrackerHash));
            result = gc.dependentTrackerHash.VisitAllEntryIndices(VisitAllEntryDependentTrackerHash<Visitor>(&visitor, &gc.dependentTrackerHash));
        }

        GCPROTECT_END();
    }

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

    if (KeyToDependentTrackersHash == NULL)
    {
        return; // Nothing was ever added, so removing all is easy
    }

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
        gc.keyToTrackersHash = GCHeapHashKeyToDependentTrackersHash((GCHEAPHASHOBJECTREF)ObjectFromHandle(KeyToDependentTrackersHash));
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
#endif // !DACCESS_COMPILE

template <class TRAITS>
void CrossLoaderAllocatorHash<TRAITS>::Init(LoaderAllocator *pAssociatedLoaderAllocator)
{
    LIMITED_METHOD_CONTRACT;
    _loaderAllocator = pAssociatedLoaderAllocator;
}

template <class TRAITS>
template <class Visitor>
/*static*/ bool CrossLoaderAllocatorHash<TRAITS>::VisitKeyValueStore(OBJECTREF *pLoaderAllocatorRef, UPTRARRAYREF *pKeyValueStore, Visitor &visitor)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    DWORD usedEntries = ((DWORD)(*pKeyValueStore)->GetDirectPointerToNonObjectElements()[1]) + 2;
    for (DWORD index = 2; index < usedEntries; ++index)
    {
        if (!visitor(*pLoaderAllocatorRef, (TKey)(*pKeyValueStore)->GetDirectPointerToNonObjectElements()[0], (TValue)(*pKeyValueStore)->GetDirectPointerToNonObjectElements()[index]))
        {
            return false;
        }
    }

    return true;
}

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

                result = VisitKeyValueStore(&gc.loaderAllocatorRef, &gc.keyValueStore, visitor);
            }
        }
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
            return VisitKeyValueStore(pLoaderAllocatorRef, _pKeyValueStore, visitor);
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
template <class Visitor>
/*static*/ bool CrossLoaderAllocatorHash<TRAITS>::VisitKeyToTrackerAllEntries(LAHASHKEYTOTRACKERSREF keyToTrackerUnsafe, Visitor &visitor)
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
        LAHASHKEYTOTRACKERSREF keyToTracker;
        OBJECTREF loaderAllocatorRef;
        UPTRARRAYREF keyValueStore;
    }gc;

    ZeroMemory(&gc, sizeof(gc));
    gc.keyToTracker = keyToTrackerUnsafe;

    bool result = true;

    GCPROTECT_BEGIN(gc);
    {
        gc.keyValueStore = gc.keyToTracker->_laLocalKeyValueStore;
        result = VisitKeyValueStore(&gc.loaderAllocatorRef, &gc.keyValueStore, visitor);
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
        LAToDependentTrackerHash = _loaderAllocator->GetDomain()->CreateHandle(laToDependentHandleHash);
        _loaderAllocator->RegisterHandleForCleanup(LAToDependentTrackerHash);
    }

    if (KeyToDependentTrackersHash == NULL)
    {
        OBJECTREF keyToDependentTrackersHash = AllocateObject(MscorlibBinder::GetClass(CLASS__GCHEAPHASH));
        KeyToDependentTrackersHash = _loaderAllocator->GetDomain()->CreateHandle(keyToDependentTrackersHash);
        _loaderAllocator->RegisterHandleForCleanup(KeyToDependentTrackersHash);
    }
}

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
        gc.dependentTrackerHash = GCHeapHashDependentHashTrackerHash((GCHEAPHASHOBJECTREF)ObjectFromHandle(LAToDependentTrackerHash));
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

template <class TRAITS>
GCHEAPHASHOBJECTREF CrossLoaderAllocatorHash<TRAITS>::GetKeyToValueCrossLAHashForHashkeyToTrackers(LAHASHKEYTOTRACKERSREF hashKeyToTrackersUnsafe, LoaderAllocator* pValueLoaderAllocator)
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
        GCHeapHashDependentHashTrackerHash dependentTrackerHash;
        LAHASHDEPENDENTHASHTRACKERREF dependentTrackerMaybe;
        LAHASHDEPENDENTHASHTRACKERREF dependentTracker;
        LAHASHKEYTOTRACKERSREF hashKeyToTrackers;
        GCHEAPHASHOBJECTREF returnValue;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.hashKeyToTrackers = hashKeyToTrackersUnsafe;
    GCPROTECT_BEGIN(gc)
    {
        // Now gc.hashKeyToTrackers is filled in.

        // Is there a single dependenttracker here, or a set, or no dependenttracker at all
        if (gc.hashKeyToTrackers->_trackerOrTrackerSet == NULL)
        {
            gc.dependentTracker = GetDependentTrackerForLoaderAllocator(pValueLoaderAllocator);
            SetObjectReference(&gc.hashKeyToTrackers->_trackerOrTrackerSet, gc.dependentTracker, GetAppDomain());
        }
        else if (gc.hashKeyToTrackers->_trackerOrTrackerSet->GetMethodTable() == MscorlibBinder::GetClass(CLASS__LAHASHDEPENDENTHASHTRACKER))
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

        // At this stage gc.dependentTracker is setup to have a good value
        gc.returnValue = gc.dependentTracker->GetDependentTarget();
    }
    GCPROTECT_END();

    return gc.returnValue;
}

#endif // !CROSSGEN_COMPILE
#endif // CROSSLOADERALLOCATORHASH_H
#endif // CROSSLOADERALLOCATORHASH_INL
