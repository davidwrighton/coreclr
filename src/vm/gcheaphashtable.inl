// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifdef GCHEAPHASHTABLE_H
#ifndef GCHEAPHASHTABLE_INL
#define GCHEAPHASHTABLE_INL

template <bool remove_supported>
/*static */bool DefaultGCHeapHashTraits<remove_supported>::IsNull(PTRARRAYREF arr, INT32 index)
{
    LIMITED_METHOD_CONTRACT;

    return arr->GetAt(index) == 0;
}

template <bool remove_supported>
/*static */bool DefaultGCHeapHashTraits<remove_supported>::IsDeleted(PTRARRAYREF arr, INT32 index, GCHEAPHASHOBJECTREF gcHeap)
{
    LIMITED_METHOD_CONTRACT;

    if (s_remove_supported)
    {
        return gcHeap == arr->GetAt(index);
    }
    else
    {
        return false;
    }
}

template <bool remove_supported>
/*static*/ typename DefaultGCHeapHashTraits<remove_supported>::THashArrayType DefaultGCHeapHashTraits<remove_supported>::AllocateArray(INT32 size)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    return (THashArrayType)AllocateObjectArray(size, g_pObjectClass);
}

    // Not a part of the traits api, but used to allow derived traits to save on code
template <bool remove_supported>
/*static*/ OBJECTREF DefaultGCHeapHashTraits<remove_supported>::GetValueAtIndex(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index)
{
    LIMITED_METHOD_CONTRACT;

    PTRARRAYREF arr((PTRARRAYREF)(*pgcHeap)->GetData());

    OBJECTREF value = arr->GetAt(index);

    return value;
}

template <bool remove_supported>
/*static*/ void DefaultGCHeapHashTraits<remove_supported>::CopyValue(THashArrayType srcArray, INT32 indexSrc, THashArrayType destinationArray, INT32 indexDest)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    if (srcArray == NULL)
        COMPlusThrow(kNullReferenceException);

    if ((INT32)srcArray->GetNumComponents() < indexSrc)
        COMPlusThrow(kIndexOutOfRangeException);

    OBJECTREF value = srcArray->GetAt(indexSrc);

    if ((INT32)destinationArray->GetNumComponents() < indexDest)
        COMPlusThrow(kIndexOutOfRangeException);

    destinationArray->SetAt(indexDest, value);
}

template <bool remove_supported>
/*static*/ void DefaultGCHeapHashTraits<remove_supported>::DeleteEntry(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    static_assert(supports_remove, "This hash doesn't support remove");

    PTRARRAYREF arr((*pgcHeap)->GetData());

    if (arr == NULL)
        COMPlusThrow(kNullReferenceException);

    if ((INT32)arr->GetNumComponents() < index)
        COMPlusThrow(kIndexOutOfRangeException);

    // The deleted sentinel is a self-pointer
    arr->SetAt(index, *pgcHeap);
}

template <bool remove_supported>
template<class TElement>
/*static*/ void DefaultGCHeapHashTraits<remove_supported>::GetElement(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, TElement& foundElement)
{
    LIMITED_METHOD_CONTRACT;

    foundElement = (TElement)GetValueAtIndex(pgcHeap, index);
}

template <bool remove_supported>
template<class TElement>
/*static*/ void DefaultGCHeapHashTraits<remove_supported>::SetElement(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, TElement& foundElement)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    PTRARRAYREF arr((PTRARRAYREF)(*pgcHeap)->GetData());

    if (arr == NULL)
        COMPlusThrow(kNullReferenceException);

    if ((INT32)arr->GetNumComponents() < index)
        COMPlusThrow(kIndexOutOfRangeException);

    arr->SetAt(index, foundElement);
}

template <class PtrTypeKey, bool supports_remove>
/*static */INT32 GCHeapHashTraitsPointerToPointerList<PtrTypeKey, supports_remove>::Hash(PtrTypeKey *pValue)
{
    LIMITED_METHOD_CONTRACT;
    return (INT32)*pValue;
}

template <class PtrTypeKey, bool supports_remove>
/*static */INT32 GCHeapHashTraitsPointerToPointerList<PtrTypeKey, supports_remove>::Hash(PTRARRAYREF arr, INT32 index)
{
    LIMITED_METHOD_CONTRACT;

    UPTRARRAYREF value = (UPTRARRAYREF)arr->GetAt(index);
    
    return (INT32)*value->GetDirectConstPointerToNonObjectElements();
}

template <class PtrTypeKey, bool supports_remove>
/*static */bool GCHeapHashTraitsPointerToPointerList<PtrTypeKey, supports_remove>::DoesEntryMatchKey(PTRARRAYREF arr, INT32 index, PtrTypeKey *pKey)
{
    LIMITED_METHOD_CONTRACT;

    UPTRARRAYREF value = (UPTRARRAYREF)arr->GetAt(index);
    UPTR uptrValue = *value->GetDirectConstPointerToNonObjectElements();

    return ((UPTR)*pKey) == uptrValue;
}

template <class TRAITS>
template<class TKey, class TValueSetter>
void GCHeapHash<TRAITS>::Insert(TKey *pKey, const TValueSetter &valueSetter)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    count_t hash = CallHash(pKey);
    count_t tableSize = _gcHeap->GetCapacity();
    count_t index = hash % tableSize; 
    count_t increment = 0; // delay computation

    while (TRUE)
    {
        THashArrayType arr((THashArrayType)(_gcHeap)->GetData());

        if (TRAITS::IsNull(arr, index) || TRAITS::IsDeleted(arr, index, _gcHeap))
        {
            if (arr == NULL)
                COMPlusThrow(kNullReferenceException);

            if ((INT32)arr->GetNumComponents() < index)
                COMPlusThrow(kIndexOutOfRangeException);

            valueSetter(arr, index);
            _gcHeap->IncrementCount();
            return;
        }
    
        if (increment == 0)
            increment = (hash % (tableSize-1)) + 1; 
    
        index += increment;
        if (index >= tableSize)
            index -= tableSize;
    }
}

template <class TRAITS>
void GCHeapHash<TRAITS>::CheckGrowth()
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    count_t tableMax = (count_t) (_gcHeap->GetCapacity() * TRAITS::s_density_factor_numerator / TRAITS::s_density_factor_denominator);
    if (_gcHeap->GetCount() == tableMax)
        Grow();
}

template <class TRAITS>
void GCHeapHash<TRAITS>::Grow()
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    THashArrayType newTable = Grow_OnlyAllocateNewTable();
    ReplaceTable(newTable);
}

template <class TRAITS>
typename GCHeapHash<TRAITS>::THashArrayType GCHeapHash<TRAITS>::Grow_OnlyAllocateNewTable()
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        INSTANCE_CHECK;
    }
    CONTRACTL_END;

    count_t newSize = (count_t) (_gcHeap->GetCount()
                                * TRAITS::s_growth_factor_numerator / TRAITS::s_growth_factor_denominator
                                * TRAITS::s_density_factor_denominator / TRAITS::s_density_factor_numerator);
    if (newSize < TRAITS::s_minimum_allocation)
        newSize = TRAITS::s_minimum_allocation;

    // handle potential overflow
    if (newSize < _gcHeap->GetCount())
        ThrowOutOfMemory();

    return TRAITS::AllocateArray(NextPrime(newSize));
}

template <class TRAITS>
bool GCHeapHash<TRAITS>::IsPrime(count_t number)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
    }
    CONTRACTL_END;

    // This is a very low-tech check for primality, which doesn't scale very well.  
    // There are more efficient tests if this proves to be burdensome for larger
    // tables.

    if ((number & 1) == 0)
        return false;

    count_t factor = 3;
    while (factor * factor <= number)
    {
        if ((number % factor) == 0)
            return false;
        factor += 2;
    }

    return true;
}

template <class TRAITS>
typename GCHeapHash<TRAITS>::count_t GCHeapHash<TRAITS>::NextPrime(count_t number)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
    }
    CONTRACTL_END;

    static_assert(sizeof(INT32) == sizeof(g_shash_primes[0]), "the cast below of g_shash_primes[] to INT32 isn't safe due to loss of precision.");

    for (int i = 0; i < (int) (sizeof(g_shash_primes) / sizeof(g_shash_primes[0])); i++) {
        if ((INT32)g_shash_primes[i] >= number)
            return (INT32)g_shash_primes[i];
    }

    if ((number&1) == 0)
        number++;

    while (number != 1) {
        if (IsPrime(number))
            return number;
        number +=2;
    }

    // overflow
    ThrowOutOfMemory();
}

template <class TRAITS>
void GCHeapHash<TRAITS>::ReplaceTable(THashArrayType newTable)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    GCPROTECT_BEGIN(newTable);
    {
        count_t newTableSize = (count_t)newTable->GetNumComponents();
        count_t oldTableSize = _gcHeap->GetCapacity();

        // Move all entries over to the new table
        count_t capacity = _gcHeap->GetCapacity();

        for (count_t index = 0; index < capacity; ++index)
        {
            THashArrayType arr((THashArrayType)(_gcHeap)->GetData());
            if (!TRAITS::IsNull(arr, index) && !TRAITS::IsDeleted(arr, index, _gcHeap))
            {
                count_t hash = CallHash(arr, index);
                count_t tableSize = (count_t)newTable->GetNumComponents();
                count_t newIndex = hash % tableSize; 
                count_t increment = 0; // delay computation

                // Value to copy is in index
                while (TRUE)
                {
                    if (TRAITS::IsNull(newTable, newIndex))
                    {
                        arr = (THashArrayType)(_gcHeap)->GetData();
                        TRAITS::CopyValue(arr, index, newTable, newIndex);
                        break;
                    }
                
                    if (increment == 0)
                        increment = (hash % (tableSize-1)) + 1; 
                
                    newIndex += increment;
                    if (newIndex >= tableSize)
                        newIndex -= tableSize;
                }
            }
        }

        _gcHeap->SetTable((BASEARRAYREF)newTable);
    }
    GCPROTECT_END();
}

template <class TRAITS>
template<class TVisitor>
bool GCHeapHash<TRAITS>::VisitAllEntryIndices(TVisitor &visitor)
{
    WRAPPER_NO_CONTRACT;

    count_t capacity = _gcHeap->GetCapacity();

    for (count_t index = 0; index < capacity; ++index)
    {
        THashArrayType arr((THashArrayType)(_gcHeap)->GetData());
        if (!TRAITS::IsNull(arr, index) && !TRAITS::IsDeleted(arr, index, _gcHeap))
        {
            if (!visitor(index))
                return false;
        }
    }

    return true;
}

template <class TRAITS>
template<class TKey, class TValueSetter>
void GCHeapHash<TRAITS>::Add(TKey *pKey, const TValueSetter &valueSetter)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    CheckGrowth();
    Insert(pKey, valueSetter);
}

    // Get the index in the hashtable of the value which matches key, or -1 if there are no matches
template <class TRAITS>
template<class TKey>
INT32 GCHeapHash<TRAITS>::GetValueIndex(TKey *pKey)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    count_t hash = CallHash(pKey);
    count_t tableSize = _gcHeap->GetCapacity();

    // If the table is empty, then 
    if (tableSize == 0)
        return -1;

    count_t index = hash % tableSize; 
    count_t increment = 0; // delay computation

    THashArrayType arr((THashArrayType)(_gcHeap)->GetData());

    while (TRUE)
    {
        if (TRAITS::IsNull(arr, index))
        {
            return -1;
        }

        if (!TRAITS::IsDeleted(arr, index, _gcHeap) && TRAITS::DoesEntryMatchKey(arr, index, pKey))
        {
            return index;
        }
    
        if (increment == 0)
            increment = (hash % (tableSize-1)) + 1; 

        index += increment;
        if (index >= tableSize)
            index -= tableSize;
    }
}

template <class TRAITS>
template<class TElement>
void GCHeapHash<TRAITS>::GetElement(INT32 index, TElement& foundElement)
{
    WRAPPER_NO_CONTRACT;

    TRAITS::GetElement(&_gcHeap, index, foundElement);
}

    // Use this to update an value within the hashtable directly. 
    // It is ONLY safe to do if the index already points at an element
    // which already exists and has the same key as the newElementValue
template <class TRAITS>
template<class TElement>
void GCHeapHash<TRAITS>::SetElement(INT32 index, TElement& newElementValue)
{
    TRAITS::SetElement(&_gcHeap, index, newElementValue);
}

template <class TRAITS>
template<class TKey>
void GCHeapHash<TRAITS>::DeleteEntry(TKey *pKey)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_COOPERATIVE;
    }
    CONTRACTL_END;

    INT32 index = GetValueIndex(pKey);
    if (index != -1)
    {
        TRAITS::DeleteEntry(&_gcHeap, index);
        _gcHeap->DecrementCount();
    }
}

#endif // GCHEAPHASHTABLE_INL
#endif // GCHEAPHASHTABLE_H
