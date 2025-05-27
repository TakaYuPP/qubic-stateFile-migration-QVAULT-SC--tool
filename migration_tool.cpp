#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "./m256.h"
#include "keyUtils.h"
#include "K12AndKeyUtil.h"
#define _A 0
#define _B 1
#define _C 2
#define _D 3
#define _E 4
#define _F 5
#define _G 6
#define _H 7
#define _I 8
#define _J 9
#define _K 10
#define _L 11
#define _M 12
#define _N 13
#define _O 14
#define _P 15
#define _Q 16
#define _R 17
#define _S 18
#define _T 19
#define _U 20
#define _V 21
#define _W 22
#define _X 23
#define _Y 24
#define _Z 25
#define ID(_00, _01, _02, _03, _04, _05, _06, _07, _08, _09, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55) _mm256_set_epi64x(((((((((((((((uint64_t)_55) * 26 + _54) * 26 + _53) * 26 + _52) * 26 + _51) * 26 + _50) * 26 + _49) * 26 + _48) * 26 + _47) * 26 + _46) * 26 + _45) * 26 + _44) * 26 + _43) * 26 + _42, ((((((((((((((uint64_t)_41) * 26 + _40) * 26 + _39) * 26 + _38) * 26 + _37) * 26 + _36) * 26 + _35) * 26 + _34) * 26 + _33) * 26 + _32) * 26 + _31) * 26 + _30) * 26 + _29) * 26 + _28, ((((((((((((((uint64_t)_27) * 26 + _26) * 26 + _25) * 26 + _24) * 26 + _23) * 26 + _22) * 26 + _21) * 26 + _20) * 26 + _19) * 26 + _18) * 26 + _17) * 26 + _16) * 26 + _15) * 26 + _14, ((((((((((((((uint64_t)_13) * 26 + _12) * 26 + _11) * 26 + _10) * 26 + _09) * 26 + _08) * 26 + _07) * 26 + _06) * 26 + _05) * 26 + _04) * 26 + _03) * 26 + _02) * 26 + _01) * 26 + _00)

typedef m256i id;
constexpr unsigned int QVAULT_MAX_NUMBER_OF_PROPOSAL = 65536;

template <typename T, uint64_t L>
struct Array
{
private:
    static_assert(L && !(L & (L - 1)),
        "The capacity of the array must be 2^N."
        );

    T _values[L];

public:
    // Return number of elements in array
    static inline constexpr uint64_t capacity()
    {
        return L;
    }

    // Get element of array
    inline const T& get(uint64_t index) const
    {
        return _values[index & (L - 1)];
    }

    // Set element of array
    inline void set(uint64_t index, const T& value)
    {
        _values[index & (L - 1)] = value;
    }

    // Set content of array by copying memory (size must match)
    template <typename AT>
    inline void setMem(const AT& value)
    {
        static_assert(sizeof(_values) == sizeof(value), "This function can only be used if the overall size of both objects match.");
        // This if is resolved at compile time
        if (sizeof(_values) == 32)
        {
            // assignment uses __m256i intrinsic CPU functions which should be very fast
            *((id*)_values) = *((id*)&value);
        }
        else
        {
            // generic copying
            copyMemory(*this, value);
        }
    }

    // Set all elements to passed value
    inline void setAll(const T& value)
    {
        for (uint64_t i = 0; i < L; ++i)
            _values[i] = value;
    }

    // Set elements in range to passed value
    inline void setRange(uint64_t indexBegin, uint64_t indexEnd, const T& value)
    {
        for (uint64_t i = indexBegin; i < indexEnd; ++i)
            _values[i & (L - 1)] = value;
    }

    // Returns true if all elements of the range equal value (and range is valid).
    inline bool rangeEquals(uint64_t indexBegin, uint64_t indexEnd, const T& value) const
    {
        if (indexEnd > L || indexBegin > indexEnd)
            return false;
        for (uint64_t i = indexBegin; i < indexEnd; ++i)
        {
            if (!(_values[i] == value))
                return false;
        }
        return true;
    }
};

void setMem(void* buffer, unsigned long long size, unsigned char value)
{
    memset(buffer, value, size);
}

// Hash function class to be used with the hash map.
template <typename KeyT> class HashFunction 
{
public:
    static uint64_t hash(const KeyT& key);
};


// Hash map of (key, value) pairs of type (KeyT, ValueT) and total element capacity L.
template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc = HashFunction<KeyT>>
class HashMap
{
private:
    static_assert(L && !(L& (L - 1)),
        "The capacity of the hash map must be 2^N."
        );
    static constexpr int64_t _nEncodedFlags = L > 32 ? 32 : L;

    // Hash map of (key, value) pairs
    struct Element
    {
        KeyT key;
        ValueT value;
    } _elements[L];

    // 2 bits per element of _elements: 0b00 = not occupied; 0b01 = occupied; 0b10 = occupied but marked for removal; 0b11 is unused
    // The state "occupied but marked for removal" is needed for finding the index of a key in the hash map. Setting an entry to
    // "not occupied" in remove() would potentially undo a collision, create a gap, and mess up the entry search.
    uint64_t _occupationFlags[(L * 2 + 63) / 64];

    uint64_t _population;
    uint64_t _markRemovalCounter;

    // Read and encode 32 POV occupation flags, return a 64bits number presents 32 occupation flags
    uint64_t _getEncodedOccupationFlags(const uint64_t* occupationFlags, const int64_t elementIndex) const;

public:
    HashMap()
    {
        reset();
    }

    // Return maximum number of elements that may be stored.
    static constexpr uint64_t capacity()
    {
        return L;
    }

    // Return overall number of elements.
    inline uint64_t population() const;

    // Return boolean indicating whether key is contained in the hash map.
    // If key is contained, write the associated value into the provided ValueT&. 
    bool get(const KeyT& key, ValueT& value) const;

    // Return index of element with key in hash map _elements, or NULL_INDEX if not found.
    int64_t getElementIndex(const KeyT& key) const;

    // Return key at elementIndex.
    inline KeyT key(int64_t elementIndex) const;

    // Return value at elementIndex.
    inline ValueT value(int64_t elementIndex) const;

    // Add element (key, value) to the hash map, return elementIndex of new element.
    // If key already exists in the hash map, the old value will be overwritten.
    // If the hash map is full, return NULL_INDEX.
    int64_t set(const KeyT& key, const ValueT& value);

    // Mark element for removal.
    void removeByIndex(int64_t elementIdx);

    // Mark element for removal if key is contained in the hash map, 
    // returning the elementIndex (or NULL_INDEX if the hash map does not contain the key).
    int64_t removeByKey(const KeyT& key);

    // Remove all elements marked for removal, this is a very expensive operation.
    void cleanup();

    // Replace value for *existing* key, do nothing otherwise.
    // - The key exists: replace its value. Return true.
    // - The key is not contained in the hash map: no action is taken. Return false.
    bool replace(const KeyT& key, const ValueT& newValue);

    // Reinitialize as empty hash map.
    void reset();
};

// Hash set of keys of type KeyT and total element capacity L.
template <typename KeyT, uint64_t L, typename HashFunc = HashFunction<KeyT>>
class HashSet
{
private:
    static_assert(L && !(L& (L - 1)),
        "The capacity of the hash set must be 2^N."
        );
    static constexpr int64_t _nEncodedFlags = L > 32 ? 32 : L;

    // Hash set
    KeyT _keys[L];

    // 2 bits per element of _elements: 0b00 = not occupied; 0b01 = occupied; 0b10 = occupied but marked for removal; 0b11 is unused
    // The state "occupied but marked for removal" is needed for finding the index of a key in the hash map. Setting an entry to
    // "not occupied" in remove() would potentially undo a collision, create a gap, and mess up the entry search.
    uint64_t _occupationFlags[(L * 2 + 63) / 64];

    uint64_t _population;
    uint64_t _markRemovalCounter;

    // Read and encode 32 POV occupation flags, return a 64bits number presents 32 occupation flags
    uint64_t _getEncodedOccupationFlags(const uint64_t* occupationFlags, const int64_t elementIndex) const;

public:
    HashSet()
    {
        reset();
    }

    // Return maximum number of elements that may be stored.
    static constexpr uint64_t capacity()
    {
        return L;
    }

    // Return overall number of elements.
    inline uint64_t population() const;

    // Return boolean indicating whether key is contained in the hash set.
    bool contains(const KeyT& key) const;

    // Return index of element with key in hash set _keys, or NULL_INDEX if not found.
    int64_t getElementIndex(const KeyT& key) const;

    // Return key at elementIndex.
    inline KeyT key(int64_t elementIndex) const;

    // Add key to the hash set, return elementIndex of new element.
    // If key already exists in the hash set, this does nothing.
    // If the hash map is full, return NULL_INDEX.
    int64_t add(const KeyT& key);

    // Mark element for removal.
    void removeByIndex(int64_t elementIdx);

    // Mark element for removal if key is contained in the hash set, 
    // returning the elementIndex (or NULL_INDEX if the hash map does not contain the key).
    int64_t remove(const KeyT& key);

    // Remove all elements marked for removal, this is an expensive operation.
    void cleanup();

    // Reinitialize as empty hash set.
    void reset();
};

template <typename KeyT>
uint64_t HashFunction<KeyT>::hash(const KeyT& key) 
{
    uint64_t ret;
    KangarooTwelve(&key, sizeof(KeyT), &ret, 8);
    return ret;
}

inline uint64_t HashFunction<m256i>::hash(const m256i& key)
	{
		return key.u64._0;
	}

//////////////////////////////////////////////////////////////////////////////
// HashMap template class

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
uint64_t HashMap<KeyT, ValueT, L, HashFunc>::_getEncodedOccupationFlags(const uint64_t* occupationFlags, const int64_t elementIndex) const
{
    const int64_t offset = (elementIndex & 31) << 1;
    uint64_t flags = occupationFlags[elementIndex >> 5] >> offset;
    if (offset > 0)
    {
        flags |= occupationFlags[((elementIndex + 32) & (L - 1)) >> 5] << (2 * _nEncodedFlags - offset);
    }
    return flags;
}


template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
bool HashMap<KeyT, ValueT, L, HashFunc>::get(const KeyT& key, ValueT& value) const 
{
    int64_t elementIndex = getElementIndex(key);
    if (elementIndex != NULL_INDEX) 
    {
        value = _elements[elementIndex].value;
        return true;
    }
    return false;
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
int64_t HashMap<KeyT, ValueT, L, HashFunc>::getElementIndex(const KeyT& key) const
{
    int64_t index = HashFunc::hash(key) & (L - 1);
    for (int64_t counter = 0; counter < L; counter += 32)
    {
        uint64_t flags = _getEncodedOccupationFlags(_occupationFlags, index);
        for (auto i = 0; i < _nEncodedFlags; i++, flags >>= 2)
        {
            switch (flags & 3ULL)
            {
            case 0:
                return NULL_INDEX;
            case 1:
                if (_elements[index].key == key)
                {
                    return index;
                }
                break;
            }
            index = (index + 1) & (L - 1);
        }
    }
    return NULL_INDEX;
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
inline KeyT HashMap<KeyT, ValueT, L, HashFunc>::key(int64_t elementIndex) const
{
    return _elements[elementIndex & (L - 1)].key;
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
inline ValueT HashMap<KeyT, ValueT, L, HashFunc>::value(int64_t elementIndex) const
{
    return _elements[elementIndex & (L - 1)].value;
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
inline uint64_t HashMap<KeyT, ValueT, L, HashFunc>::population() const
{
    return _population;
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
int64_t HashMap<KeyT, ValueT, L, HashFunc>::set(const KeyT& key, const ValueT& value)
{
    if (_population < capacity() && _markRemovalCounter < capacity())
    {
        // search in hash map
        int64_t index = HashFunc::hash(key) & (L - 1);
        for (int64_t counter = 0; counter < L; counter += 32)
        {
            uint64_t flags = _getEncodedOccupationFlags(_occupationFlags, index);
            for (auto i = 0; i < _nEncodedFlags; i++, flags >>= 2)
            {
                switch (flags & 3ULL)
                {
                case 0:
                    // empty entry -> put element and mark as occupied
                    _occupationFlags[index >> 5] |= (1ULL << ((index & 31) << 1));
                    _elements[index].key = key;
                    _elements[index].value = value;
                    _population++;
                    return index;
                case 1:
                    if (_elements[index].key == key)
                    {
                        // found key -> insert new value
                        _elements[index].value = value;
                        return index;
                    }
                    break;
                // TODO: fill gaps of "marked for removal!? -> should remove check in cleanup, because cleanup can still speed-up access even if gaps were reused
                }
                index = (index + 1) & (L - 1);
            }
        }
    }
    else if (_population == capacity())
    {
        // Check if key exists for value replacement.
        int64_t index = getElementIndex(key);
        if (index != NULL_INDEX)
        {
            _elements[index].value = value;
            return index;
        }
    }
    return NULL_INDEX;
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
void HashMap<KeyT, ValueT, L, HashFunc>::removeByIndex(int64_t elementIdx)
{
    elementIdx &= (L - 1);
    uint64_t flags = _getEncodedOccupationFlags(_occupationFlags, elementIdx);

    if ((flags & 3ULL) == 1)
    {
        _population--;
        _markRemovalCounter++;
        _occupationFlags[elementIdx >> 5] ^= (3ULL << ((elementIdx & 31) << 1));

        const bool CLEAR_UNUSED_ELEMENT = true;
        if (CLEAR_UNUSED_ELEMENT)
        {
            setMem(&_elements[elementIdx], sizeof(Element), 0);
        }
    }
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
int64_t HashMap<KeyT, ValueT, L, HashFunc>::removeByKey(const KeyT& key) 
{
    int64_t elementIndex = getElementIndex(key);
    if (elementIndex == NULL_INDEX) 
    {
        return NULL_INDEX;
    }
    else 
    {
        removeByIndex(elementIndex);
        return elementIndex;
    }
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
void HashMap<KeyT, ValueT, L, HashFunc>::cleanup()
{
    // _elements gets occupied over time with entries of type 3 which means they are marked for cleanup.
    // Once cleanup is called it's necessary to remove all these type 3 entries by reconstructing a fresh hash map residing in scratchpad buffer.
    // Cleanup() called for a hash map having only type 3 entries must give the result equal to reset() memory content wise.

    // Quick check to cleanup
    if (!_markRemovalCounter)
    {
        return;
    }

    // Speedup case of empty hash map but existed marked for removal elements
    if (!population())
    {
        reset();
        return;
    }

    // Init buffers
    auto* _elementsBuffer = reinterpret_cast<Element*>(::__scratchpad());
    auto* _occupationFlagsBuffer = reinterpret_cast<uint64_t*>(_elementsBuffer + L);
    auto* _stackBuffer = reinterpret_cast<int64_t*>(
        _occupationFlagsBuffer + sizeof(_occupationFlags) / sizeof(_occupationFlags[0]));
    setMem(::__scratchpad(), sizeof(_elements) + sizeof(_occupationFlags), 0);
    uint64_t newPopulation = 0;

    // Go through hash map. For each element that is occupied but not marked for removal, insert element in new hash map's buffers.
    constexpr uint64_t oldIndexGroupCount = (L >> 5) ? (L >> 5) : 1;
    for (int64_t oldIndexGroup = 0; oldIndexGroup < oldIndexGroupCount; oldIndexGroup++)
    {
        const uint64_t flags = _occupationFlags[oldIndexGroup];
        uint64_t maskBits = (0xAAAAAAAAAAAAAAAA & (flags << 1));
        maskBits &= maskBits ^ (flags & 0xAAAAAAAAAAAAAAAA);
        int64_t oldIndexOffset = _tzcnt_u64(maskBits) & 0xFE;
        const int64_t oldIndexOffsetEnd = 64 - (_lzcnt_u64(maskBits) & 0xFE);
        for (maskBits >>= oldIndexOffset;
            oldIndexOffset < oldIndexOffsetEnd; oldIndexOffset += 2, maskBits >>= 2)
        {
            // Only add elements to new hash map that are occupied and not marked for removal
            if (maskBits & 3ULL)
            {
                // find empty position in new hash map
                const int64_t oldIndex = (oldIndexGroup << 5) + (oldIndexOffset >> 1);
                int64_t newIndex = HashFunc::hash(_elements[oldIndex].key) & (L - 1);
                for (int64_t counter = 0; counter < L; counter += 32)
                {
                    uint64_t newFlags = _getEncodedOccupationFlags(_occupationFlagsBuffer, newIndex);
                    for (int64_t i = 0; i < _nEncodedFlags; i++, newFlags >>= 2)
                    {
                        if ((newFlags & 3ULL) == 0)
                        {
                            newIndex = (newIndex + i) & (L - 1);
                            goto foundEmptyPosition;
                        }
                    }
                    newIndex = (newIndex + _nEncodedFlags) & (L - 1);
                }
#ifdef NO_UEFI
                // should never be reached, because old and new map have same capacity (there should always be an empty slot)
                goto cleanupBug;
#endif

            foundEmptyPosition:
                // occupy empty hash map entry
                _occupationFlagsBuffer[newIndex >> 5] |= (1ULL << ((newIndex & 31) << 1));
                copyMem(&_elementsBuffer[newIndex], &_elements[oldIndex], sizeof(Element));

                // check if we are done
                newPopulation += 1;
                if (newPopulation == _population)
                {
                    // all elements have been transferred -> overwrite old array with new array
                    copyMem(_elements, _elementsBuffer, sizeof(_elements));
                    copyMem(_occupationFlags, _occupationFlagsBuffer, sizeof(_occupationFlags));
                    _markRemovalCounter = 0;
                    return;
                }
            }
        }
    }

#ifdef NO_UEFI
    cleanupBug :
    // don't expect here, certainly got error!!!
    printf("ERROR: Something went wrong at cleanup!\n");
#endif
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
bool HashMap<KeyT, ValueT, L, HashFunc>::replace(const KeyT& key, const ValueT& newValue)
{
    int64_t elementIndex = getElementIndex(key);
    if (elementIndex != NULL_INDEX) 
    {
        _elements[elementIndex].value = newValue;
        return true;
    }
    return false;
}

template <typename KeyT, typename ValueT, uint64_t L, typename HashFunc>
void HashMap<KeyT, ValueT, L, HashFunc>::reset()
{
    setMem(this, sizeof(*this), 0);
}


//////////////////////////////////////////////////////////////////////////////
	// HashSet template class

template <typename KeyT, uint64_t L, typename HashFunc>
uint64_t HashSet<KeyT, L, HashFunc>::_getEncodedOccupationFlags(const uint64_t* occupationFlags, const int64_t elementIndex) const
{
    const int64_t offset = (elementIndex & 31) << 1;
    uint64_t flags = occupationFlags[elementIndex >> 5] >> offset;
    if (offset > 0)
    {
        flags |= occupationFlags[((elementIndex + 32) & (L - 1)) >> 5] << (2 * _nEncodedFlags - offset);
    }
    return flags;
}

template <typename KeyT, uint64_t L, typename HashFunc>
bool HashSet<KeyT, L, HashFunc>::contains(const KeyT& key) const
{
    return getElementIndex(key) != NULL_INDEX;
}

template <typename KeyT, uint64_t L, typename HashFunc>
int64_t HashSet<KeyT, L, HashFunc>::getElementIndex(const KeyT& key) const
{
    int64_t index = HashFunc::hash(key) & (L - 1);
    for (int64_t counter = 0; counter < L; counter += 32)
    {
        uint64_t flags = _getEncodedOccupationFlags(_occupationFlags, index);
        for (auto i = 0; i < _nEncodedFlags; i++, flags >>= 2)
        {
            switch (flags & 3ULL)
            {
            case 0:
                return NULL_INDEX;
            case 1:
                if (_keys[index] == key)
                {
                    return index;
                }
                break;
            }
            index = (index + 1) & (L - 1);
        }
    }
    return NULL_INDEX;
}

template <typename KeyT, uint64_t L, typename HashFunc>
inline KeyT HashSet<KeyT, L, HashFunc>::key(int64_t elementIndex) const
{
    return _keys[elementIndex & (L - 1)];
}

template <typename KeyT, uint64_t L, typename HashFunc>
inline uint64_t HashSet<KeyT, L, HashFunc>::population() const
{
    return _population;
}

template <typename KeyT, uint64_t L, typename HashFunc>
int64_t HashSet<KeyT, L, HashFunc>::add(const KeyT& key)
{
    if (_population < capacity() && _markRemovalCounter < capacity())
    {
        // search in hash map
        int64_t index = HashFunc::hash(key) & (L - 1);
        for (int64_t counter = 0; counter < L; counter += 32)
        {
            uint64_t flags = _getEncodedOccupationFlags(_occupationFlags, index);
            for (auto i = 0; i < _nEncodedFlags; i++, flags >>= 2)
            {
                switch (flags & 3ULL)
                {
                case 0:
                    // empty entry -> put element and mark as occupied
                    _occupationFlags[index >> 5] |= (1ULL << ((index & 31) << 1));
                    _keys[index] = key;
                    _population++;
                    return index;
                case 1:
                    // used entry
                    if (_keys[index] == key)
                    {
                        // found key -> return index
                        return index;
                    }
                    break;
                case 2:
                    // marked for removal -> reuse slot (put key and set flags from 2 to 1)
                    _occupationFlags[index >> 5] ^= (3ULL << ((index & 31) << 1));
                    _keys[index] = key;
                    _population++;
                    ASSERT(_markRemovalCounter > 0);
                    _markRemovalCounter--;
                    return index;
                }
                index = (index + 1) & (L - 1);
            }
        }
    }
    else if (_population == capacity())
    {
        // Check if key exists.
        int64_t index = getElementIndex(key);
        if (index != NULL_INDEX)
        {
            return index;
        }
    }
    return NULL_INDEX;
}

template <typename KeyT, uint64_t L, typename HashFunc>
void HashSet<KeyT, L, HashFunc>::removeByIndex(int64_t elementIdx)
{
    elementIdx &= (L - 1);
    uint64_t flags = _getEncodedOccupationFlags(_occupationFlags, elementIdx);

    if ((flags & 3ULL) == 1)
    {
        _population--;
        _markRemovalCounter++;
        _occupationFlags[elementIdx >> 5] ^= (3ULL << ((elementIdx & 31) << 1));

        const bool CLEAR_UNUSED_ELEMENT = true;
        if (CLEAR_UNUSED_ELEMENT)
        {
            setMem(&_keys[elementIdx], sizeof(KeyT), 0);
        }
    }
}

template <typename KeyT, uint64_t L, typename HashFunc>
int64_t HashSet<KeyT, L, HashFunc>::remove(const KeyT& key)
{
    int64_t elementIndex = getElementIndex(key);
    if (elementIndex == NULL_INDEX)
    {
        return NULL_INDEX;
    }
    else
    {
        removeByIndex(elementIndex);
        return elementIndex;
    }
}

template <typename KeyT, uint64_t L, typename HashFunc>
void HashSet<KeyT, L, HashFunc>::cleanup()
{
    // _keys gets occupied over time with entries of type 3 which means they are marked for cleanup.
    // Once cleanup is called it's necessary to remove all these type 3 entries by reconstructing a fresh hash map residing in scratchpad buffer.
    // Cleanup() called for a hash map having only type 3 entries must give the result equal to reset() memory content wise.

    // Speedup case of empty hash map but existed marked for removal elements
    if (!population())
    {
        reset();
        return;
    }

    // Init buffers
    auto* _keyBuffer = reinterpret_cast<KeyT*>(::__scratchpad());
    auto* _occupationFlagsBuffer = reinterpret_cast<uint64_t*>(_keyBuffer + L);
    auto* _stackBuffer = reinterpret_cast<int64_t*>(
        _occupationFlagsBuffer + sizeof(_occupationFlags) / sizeof(_occupationFlags[0]));
    setMem(::__scratchpad(), sizeof(_keys) + sizeof(_occupationFlags), 0);
    uint64_t newPopulation = 0;

    // Go through hash map. For each element that is occupied but not marked for removal, insert element in new hash map's buffers.
    constexpr uint64_t oldIndexGroupCount = (L >> 5) ? (L >> 5) : 1;
    for (int64_t oldIndexGroup = 0; oldIndexGroup < oldIndexGroupCount; oldIndexGroup++)
    {
        const uint64_t flags = _occupationFlags[oldIndexGroup];
        uint64_t maskBits = (0xAAAAAAAAAAAAAAAA & (flags << 1));
        maskBits &= maskBits ^ (flags & 0xAAAAAAAAAAAAAAAA);
        int64_t oldIndexOffset = _tzcnt_u64(maskBits) & 0xFE;
        const int64_t oldIndexOffsetEnd = 64 - (_lzcnt_u64(maskBits) & 0xFE);
        for (maskBits >>= oldIndexOffset;
            oldIndexOffset < oldIndexOffsetEnd; oldIndexOffset += 2, maskBits >>= 2)
        {
            // Only add elements to new hash map that are occupied and not marked for removal
            if (maskBits & 3ULL)
            {
                // find empty position in new hash map
                const int64_t oldIndex = (oldIndexGroup << 5) + (oldIndexOffset >> 1);
                int64_t newIndex = HashFunc::hash(_keys[oldIndex]) & (L - 1);
                for (int64_t counter = 0; counter < L; counter += 32)
                {
                    QPI::uint64_t newFlags = _getEncodedOccupationFlags(_occupationFlagsBuffer, newIndex);
                    for (int64_t i = 0; i < _nEncodedFlags; i++, newFlags >>= 2)
                    {
                        if ((newFlags & 3ULL) == 0)
                        {
                            newIndex = (newIndex + i) & (L - 1);
                            goto foundEmptyPosition;
                        }
                    }
                    newIndex = (newIndex + _nEncodedFlags) & (L - 1);
                }
#ifdef NO_UEFI
					// should never be reached, because old and new map have same capacity (there should always be an empty slot)
					goto cleanupBug;
#endif

            foundEmptyPosition:
                // occupy empty hash map entry
                _occupationFlagsBuffer[newIndex >> 5] |= (1ULL << ((newIndex & 31) << 1));
                _keyBuffer[newIndex] = _keys[oldIndex];

                // check if we are done
                newPopulation += 1;
                if (newPopulation == _population)
                {
                    // all elements have been transferred -> overwrite old array with new array
                    copyMem(_keys, _keyBuffer, sizeof(_keys));
                    copyMem(_occupationFlags, _occupationFlagsBuffer, sizeof(_occupationFlags));
                    _markRemovalCounter = 0;
                    return;
                }
            }
        }
    }

#ifdef NO_UEFI
    cleanupBug :
    // don't expect here, certainly got error!!!
    printf("ERROR: Something went wrong at cleanup!\n");
#endif
}

template <typename KeyT, uint64_t L, typename HashFunc>
void HashSet<KeyT, L, HashFunc>::reset()
{
    setMem(this, sizeof(*this), 0);
}

#pragma pack(push, 1)
struct stakingInfo
{
    id stakerAddress;
    unsigned int amount;
};
#pragma pack(pop)
stakingInfo staker[1048576];
stakingInfo votingPower[1048576];
#pragma pack(push, 1)
struct GPInfo                   // General proposal
{
    id proposer;
    unsigned int currentTotalVotingPower;
    unsigned int numberOfYes;
    unsigned int numberOfNo;
    unsigned int proposedEpoch;
    unsigned int currentQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};
#pragma pack(pop)
GPInfo GP[QVAULT_MAX_NUMBER_OF_PROPOSAL];
#pragma pack(push, 1)
struct QCPInfo                   // Quorum change proposal
{
    id proposer;
    unsigned int currentTotalVotingPower;
    unsigned int numberOfYes;
    unsigned int numberOfNo;
    unsigned int proposedEpoch;
    unsigned int currentQuorumPercent;
    unsigned int newQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};
#pragma pack(pop)
QCPInfo QCP[QVAULT_MAX_NUMBER_OF_PROPOSAL];
#pragma pack(push, 1)
struct IPOPInfo         // IPO participation
{
    id proposer;
    uint64_t totalWeight;
    uint64_t assignedFund;
    unsigned int currentTotalVotingPower;
    unsigned int numberOfYes;
    unsigned int numberOfNo;
    unsigned int proposedEpoch;
    unsigned int ipoContractIndex;
    unsigned int currentQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient invest funds.
};
#pragma pack(pop)
IPOPInfo IPOP[QVAULT_MAX_NUMBER_OF_PROPOSAL];
#pragma pack(push, 1)
struct QEarnPInfo       // Qearn participation proposal
{
    id proposer;
    uint64_t amountOfInvestPerEpoch;
    uint64_t assignedFundPerEpoch;
    unsigned int currentTotalVotingPower;
    unsigned int numberOfYes;
    unsigned int numberOfNo;
    unsigned int proposedEpoch;
    unsigned int currentQuorumPercent;
    uint8_t numberOfEpoch;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient funds.
};
#pragma pack(pop)
QEarnPInfo QEarnP[QVAULT_MAX_NUMBER_OF_PROPOSAL];
#pragma pack(push, 1)
struct FundPInfo            // Fundraising proposal
{
    id proposer;
    uint64_t pricePerOneQcap;
    unsigned int currentTotalVotingPower;
    unsigned int numberOfYes;
    unsigned int numberOfNo;
    unsigned int amountOfQcap;
    unsigned int restSaleAmount;
    unsigned int proposedEpoch;
    unsigned int currentQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};
#pragma pack(pop)
FundPInfo FundP[QVAULT_MAX_NUMBER_OF_PROPOSAL];
#pragma pack(push, 1)
struct MKTPInfo                 //  Marketplace proposal
{
    id proposer;
    uint64_t amountOfQubic;
    uint64_t shareName;
    unsigned int currentTotalVotingPower;
    unsigned int numberOfYes;
    unsigned int numberOfNo;
    unsigned int amountOfQcap;
    unsigned int currentQuorumPercent;
    unsigned int proposedEpoch;
    unsigned int shareIndex;
    unsigned int amountOfShare;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient funds. 4 is the insufficient Qcap.
};
#pragma pack(pop)
MKTPInfo MKTP[QVAULT_MAX_NUMBER_OF_PROPOSAL];
#pragma pack(push, 1)
struct AlloPInfo
{
    id proposer;
    unsigned int currentTotalVotingPower;
    unsigned int numberOfYes;
    unsigned int numberOfNo;
    unsigned int proposedEpoch;
    unsigned int currentQuorumPercent;
    unsigned int reinvested;
    unsigned int distributed;
    unsigned int team;
    unsigned int burnQcap;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};
#pragma pack(pop)
AlloPInfo AlloP[QVAULT_MAX_NUMBER_OF_PROPOSAL];
#pragma pack(push, 1)
struct MSPInfo
{
    id proposer;
    unsigned int currentTotalVotingPower;
    unsigned int numberOfYes;
    unsigned int numberOfNo;
    unsigned int proposedEpoch;
    unsigned int muslimShareIndex;
    unsigned int currentQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};
#pragma pack(pop)
MSPInfo MSP[1024];

id QCAP_ISSUER;
id reinvestingAddress;
id adminAddress;
HashSet<id, 1048576> muslim;
#pragma pack(push, 1)
struct voteStatusInfo
{
    unsigned int proposalId;
    uint8_t proposalType;
};
#pragma pack(pop)
HashMap<id, Array<voteStatusInfo, 16>, 1048576> vote;
HashMap<id, uint8_t, 1048576> countOfVote;

uint64_t proposalCreateFund, reinvestingFund, totalNotMSRevenue, totalMuslimRevenue, fundForBurn, totalHistoryRevenue, rasiedFundByQcap, lastRoundPriceOfQcap, revenueByQearn;
Array<uint64_t, 65536> revenueInQcapPerEpoch;
Array<uint64_t, 65536> revenueForOneQcapPerEpoch;
Array<uint64_t, 65536> revenueForOneMuslimPerEpoch;
Array<uint64_t, 65536> revenueForOneQvaultPerEpoch;
Array<uint64_t, 65536> revenueForReinvestPerEpoch;
Array<uint64_t, 1024> revenuePerShare;
Array<unsigned int, 64> muslimShares;
Array<unsigned int, 65536> burntQcapAmPerEpoch;
unsigned int totalVotingPower, totalStakedQcapAmount, qcapSoldAmount;
unsigned int shareholderDividend, QCAPHolderPermille, reinvestingPermille, devPermille, burnPermille, qcapBurnPermille, totalQcapBurntAmount;
unsigned int numberOfStaker, numberOfVotingPower;
unsigned int numberOfGP;
unsigned int numberOfQCP;
unsigned int numberOfIPOP;
unsigned int numberOfQEarnP;
unsigned int numberOfFundP;
unsigned int numberOfMKTP;
unsigned int numberOfAlloP;
unsigned int numberOfMSP;
unsigned int transferRightsFee;
unsigned int numberOfMuslimShare;
int numberOfMuslim;
unsigned int quorumPercent;

// Function to write new state to a file
void writeNewState(const std::string& filename) {
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        throw std::runtime_error("Failed to open the new state file.");
    }

    QCAP_ISSUER = ID(_Q, _C, _A, _P, _W, _M, _Y, _R, _S, _H, _L, _B, _J, _H, _S, _T, _T, _Z, _Q, _V, _C, _I, _B, _A, _R, _V, _O, _A, _S, _K, _D, _E, _N, _A, _S, _A, _K, _N, _O, _B, _R, _G, _P, _F, _W, _W, _K, _R, _C, _U, _V, _U, _A, _X, _Y, _E);
    adminAddress = ID(_H, _E, _C, _G, _U, _G, _H, _C, _J, _K, _Q, _O, _S, _D, _T, _M, _E, _H, _Q, _Y, _W, _D, _D, _T, _L, _F, _D, _A, _S, _Z, _K, _M, _G, _J, _L, _S, _R, _C, _S, _T, _H, _H, _A, _P, _P, _E, _D, _L, _G, _B, _L, _X, _J, _M, _N, _D);
    qcapSoldAmount = 1652235;
    transferRightsFee = 100;
    quorumPercent = 670;
    devPermille = 20;
    qcapBurnPermille = 0;
    burnPermille = 0;
    QCAPHolderPermille = 500;
    reinvestingPermille = 450;
    shareholderDividend = 30;

    outfile.write(reinterpret_cast<const char*>(&staker), sizeof(staker));
    outfile.write(reinterpret_cast<const char*>(&votingPower), sizeof(votingPower));
    outfile.write(reinterpret_cast<const char*>(&GP), sizeof(GP));
    outfile.write(reinterpret_cast<const char*>(&QCP), sizeof(QCP));
    outfile.write(reinterpret_cast<const char*>(&IPOP), sizeof(IPOP));
    outfile.write(reinterpret_cast<const char*>(&QEarnP), sizeof(QEarnP));
    outfile.write(reinterpret_cast<const char*>(&FundP), sizeof(FundP));
    outfile.write(reinterpret_cast<const char*>(&MKTP), sizeof(MKTP));
    outfile.write(reinterpret_cast<const char*>(&AlloP), sizeof(AlloP));
    outfile.write(reinterpret_cast<const char*>(&MSP), sizeof(MSP));
    outfile.write(reinterpret_cast<const char*>(&QCAP_ISSUER), sizeof(QCAP_ISSUER));
    outfile.write(reinterpret_cast<const char*>(&reinvestingAddress), sizeof(reinvestingAddress));
    outfile.write(reinterpret_cast<const char*>(&adminAddress), sizeof(adminAddress));
    outfile.write(reinterpret_cast<const char*>(&muslim), sizeof(muslim));
    outfile.write(reinterpret_cast<const char*>(&vote), sizeof(vote));
    outfile.write(reinterpret_cast<const char*>(&countOfVote), sizeof(countOfVote));
    outfile.write(reinterpret_cast<const char*>(&proposalCreateFund), sizeof(proposalCreateFund));
    outfile.write(reinterpret_cast<const char*>(&reinvestingFund), sizeof(reinvestingFund));
    outfile.write(reinterpret_cast<const char*>(&totalNotMSRevenue), sizeof(totalNotMSRevenue));
    outfile.write(reinterpret_cast<const char*>(&totalMuslimRevenue), sizeof(totalMuslimRevenue));
    outfile.write(reinterpret_cast<const char*>(&fundForBurn), sizeof(fundForBurn));
    outfile.write(reinterpret_cast<const char*>(&totalHistoryRevenue), sizeof(totalHistoryRevenue));
    outfile.write(reinterpret_cast<const char*>(&rasiedFundByQcap), sizeof(rasiedFundByQcap));
    outfile.write(reinterpret_cast<const char*>(&lastRoundPriceOfQcap), sizeof(lastRoundPriceOfQcap));
    outfile.write(reinterpret_cast<const char*>(&revenueByQearn), sizeof(revenueByQearn));
    outfile.write(reinterpret_cast<const char*>(&revenueInQcapPerEpoch), sizeof(revenueInQcapPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenueForOneQcapPerEpoch), sizeof(revenueForOneQcapPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenueForOneMuslimPerEpoch), sizeof(revenueForOneMuslimPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenueForOneQvaultPerEpoch), sizeof(revenueForOneQvaultPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenueForReinvestPerEpoch), sizeof(revenueForReinvestPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenuePerShare), sizeof(revenuePerShare));
    outfile.write(reinterpret_cast<const char*>(&muslimShares), sizeof(muslimShares));
    outfile.write(reinterpret_cast<const char*>(&burntQcapAmPerEpoch), sizeof(burntQcapAmPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&totalVotingPower), sizeof(totalVotingPower));
    outfile.write(reinterpret_cast<const char*>(&totalStakedQcapAmount), sizeof(totalStakedQcapAmount));
    outfile.write(reinterpret_cast<const char*>(&qcapSoldAmount), sizeof(qcapSoldAmount));
    outfile.write(reinterpret_cast<const char*>(&shareholderDividend), sizeof(shareholderDividend));
    outfile.write(reinterpret_cast<const char*>(&QCAPHolderPermille), sizeof(QCAPHolderPermille));
    outfile.write(reinterpret_cast<const char*>(&reinvestingPermille), sizeof(reinvestingPermille));
    outfile.write(reinterpret_cast<const char*>(&devPermille), sizeof(devPermille));
    outfile.write(reinterpret_cast<const char*>(&burnPermille), sizeof(burnPermille));
    outfile.write(reinterpret_cast<const char*>(&qcapBurnPermille), sizeof(qcapBurnPermille));
    outfile.write(reinterpret_cast<const char*>(&totalQcapBurntAmount), sizeof(totalQcapBurntAmount));
    outfile.write(reinterpret_cast<const char*>(&numberOfStaker), sizeof(numberOfStaker));
    outfile.write(reinterpret_cast<const char*>(&numberOfVotingPower), sizeof(numberOfVotingPower));
    outfile.write(reinterpret_cast<const char*>(&numberOfGP), sizeof(numberOfGP));
    outfile.write(reinterpret_cast<const char*>(&numberOfQCP), sizeof(numberOfQCP));
    outfile.write(reinterpret_cast<const char*>(&numberOfIPOP), sizeof(numberOfIPOP));
    outfile.write(reinterpret_cast<const char*>(&numberOfQEarnP), sizeof(numberOfQEarnP));
    outfile.write(reinterpret_cast<const char*>(&numberOfFundP), sizeof(numberOfFundP));
    outfile.write(reinterpret_cast<const char*>(&numberOfMKTP), sizeof(numberOfMKTP));
    outfile.write(reinterpret_cast<const char*>(&numberOfAlloP), sizeof(numberOfAlloP));
    outfile.write(reinterpret_cast<const char*>(&numberOfMSP), sizeof(numberOfMSP));
    outfile.write(reinterpret_cast<const char*>(&transferRightsFee), sizeof(transferRightsFee));
    outfile.write(reinterpret_cast<const char*>(&numberOfMuslimShare), sizeof(numberOfMuslimShare));
    outfile.write(reinterpret_cast<const char*>(&numberOfMuslim), sizeof(numberOfMuslim));
    outfile.write(reinterpret_cast<const char*>(&quorumPercent), sizeof(quorumPercent));

    if (!outfile) {
        throw std::runtime_error("Failed to write id to the file.");
    }
    outfile.close();
}

int main() {
    try {
        // File paths
        const std::string newStateFile = "contract0010.164";

        // Write the new state to a file
        writeNewState(newStateFile);

        std::cout << "Migration completed successfully. New state saved to: " << newStateFile << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}