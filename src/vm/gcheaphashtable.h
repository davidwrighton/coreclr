// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef GCHEAPHASHTABLE_H
#define GCHEAPHASHTABLE_H

class GCHeapHashObject;

template <bool remove_supported>
struct DefaultGCHeapHashTraits
{
    typedef typename PTRARRAYREF THashArrayType;
    static const INT32 s_growth_factor_numerator = 3;
    static const INT32 s_growth_factor_denominator = 2;

    static const INT32 s_density_factor_numerator = 3;
    static const INT32 s_density_factor_denominator = 4;

    static const INT32 s_minimum_allocation = 7;

    static const bool s_remove_supported = remove_supported;

    static bool IsNull(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index);
    static bool IsDeleted(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index);
    static THashArrayType AllocateArray(INT32 size);

    // Not a part of the traits api, but used to allow derived traits to save on code
    static OBJECTREF GetValueAtIndex(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index);

    static void CopyValue(GCHEAPHASHOBJECTREF *pgcHeapSrc, INT32 indexSrc, THashArrayType destinationArray, INT32 indexDest);
    static void DeleteEntry(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index);

    template<class TElement>
    static void GetElement(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, TElement& foundElement);

    template<class TElement>
    static void SetElement(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, TElement& foundElement);
};

template <class PtrTypeKey, bool supports_remove>
struct GCHeapHashTraitsPointerToPointerList : public DefaultGCHeapHashTraits<supports_remove>
{
    static INT32 Hash(PtrTypeKey *pValue);
    static INT32 Hash(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index);
    static bool DoesEntryMatchKey(GCHEAPHASHOBJECTREF *pgcHeap, INT32 index, PtrTypeKey *pKey);
};

template <class TRAITS>
class GCHeapHash
{
    GCHEAPHASHOBJECTREF _gcHeap;

    typedef typename TRAITS::THashArrayType THashArrayType;
    typedef INT32 count_t;

    private:
    // Insert into hashtable without growing. GCHEAPHASHOBJECTREF must be GC protected as must be TKey if needed
    template<class TKey, class TValueSetter>
    void Insert(TKey *pKey, const TValueSetter &valueSetter);
    void CheckGrowth();
    void Grow();
    THashArrayType Grow_OnlyAllocateNewTable();

    bool IsPrime(count_t number);
    count_t NextPrime(count_t number);

    void ReplaceTable(THashArrayType newTable);

    public:

    template<class TVisitor>
    bool VisitAllEntryIndices(TVisitor &visitor);

    template<class TKey, class TValueSetter>
    void Add(TKey *pKey, const TValueSetter &valueSetter);

    // Get the index in the hashtable of the value which matches key, or -1 if there are no matches
    template<class TKey>
    INT32 GetValueIndex(TKey *pKey);

    template<class TElement>
    void GetElement(INT32 index, TElement& foundElement);

    // Use this to update an value within the hashtable directly. 
    // It is ONLY safe to do if the index already points at an element
    // which already exists and has the same key as the newElementValue
    template<class TElement>
    void SetElement(INT32 index, TElement& newElementValue);

    template<class TKey>
    void DeleteEntry(TKey *pKey);

    GCHeapHash(GCHEAPHASHOBJECTREF gcHeap) : _gcHeap(gcHeap) {}
    GCHeapHash(OBJECTREF gcHeap) : _gcHeap((GCHEAPHASHOBJECTREF)gcHeap) {}
    GCHeapHash() : _gcHeap(TADDR(NULL)) {}
};

#endif // GCHEAPHASHTABLE_H