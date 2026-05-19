#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <immintrin.h>
#include "./m256.h"
#include "./test_utils.h"

#define NO_UEFI 1

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error("Assertion failed: " #condition); \
        } \
    } while (0)

inline void __markContractStateDirty(unsigned int) {}

template <typename Dst, typename Src>
inline void copyMemory(Dst& dst, const Src& src)
{
    static_assert(sizeof(Dst) == sizeof(Src), "copyMemory size mismatch");
    std::memcpy(&dst, &src, sizeof(Dst));
}

inline void copyMem(void* dst, const void* src, unsigned long long size)
{
    std::memcpy(dst, src, static_cast<std::size_t>(size));
}

// Stand-in for contract scratchpad (EFI stack buffer in production).
struct __ScopedScratchpad
{
    void* ptr = nullptr;
    std::vector<unsigned char> storage;

    __ScopedScratchpad(unsigned long long size, bool initZero)
    {
        storage.resize(static_cast<std::size_t>(size));
        ptr = storage.empty() ? nullptr : storage.data();
        if (initZero && ptr)
        {
            std::memset(ptr, 0, storage.size());
        }
    }
};

struct bit
	{
		bit(bool v = false) : charValue(v)
		{
		}

		operator bool() const
		{
			return !!charValue;
		}

		char charValue;
	};

	typedef signed char sint8;
	typedef unsigned char uint8;
	typedef signed short sint16;
	typedef unsigned short uint16;
	typedef signed int sint32;
	typedef unsigned int uint32;
	typedef signed long long sint64;
	typedef unsigned long long uint64;

namespace QPI
{
    using uint64 = ::uint64;
}

	// Error codes for inter-contract calls (used when calling other contracts fails)
	// These are returned to the calling contract so it can handle the error
	enum InterContractCallError : uint8
	{
		NoCallError = 0,
		CallErrorContractInErrorState = 1,      // Called contract is already in error state
		CallErrorInsufficientFees = 2,          // Called contract has no execution fee reserve
		CallErrorAllocationFailed = 3,          // Failed to allocate context on stack
		CallErrorContractInactive = 4,			// Called contract has been inactive
	};

	typedef m256i id;

#define STATIC_ASSERT(condition, identifier) static_assert(condition, #identifier);

#define NULL_ID id::zero()

	constexpr sint64 NULL_INDEX = -1;

	constexpr sint64 INVALID_AMOUNT = 0x8000000000000000;

	// Characters for building strings (for example in constructor of id / m256i)
	namespace Ch
	{
		enum : char
		{
			null = 0,
			space = ' ', slash = '/', backslash = '\\', dot = '.', comma = ',', colon = ':', semicolon = ';',
			underscore = '_', minus = '-', plus = '+', star = '*', dollar = '$', question_mark = '?', exclamation_mark = '!',
			a = 'a', b = 'b', c = 'c', d = 'd', e = 'e', f = 'f', g = 'g', h = 'h', i = 'i', j = 'j', k = 'k', l = 'l', m = 'm',
			n = 'n', o = 'o', p = 'p', q = 'q', r = 'r', s = 's', t = 't', u = 'u', v = 'v', w = 'w', x = 'x', y = 'y', z = 'z',
			A = 'A', B = 'B', C = 'C', D = 'D', E = 'E', F = 'F', G = 'G', H = 'H', I = 'I', J = 'J', K = 'K', L = 'L', M = 'M',
			N = 'N', O = 'O', P = 'P', Q = 'Q', R = 'R', S = 'S', T = 'T', U = 'U', V = 'V', W = 'W', X = 'X', Y = 'Y', Z = 'Z',
			_0 = '0', _1 = '1', _2 = '2', _3 = '3', _4 = '4', _5 = '5', _6 = '6', _7 = '7', _8 = '8', _9 = '9',
		};
	}

	// Wrapper around a contract's entire state struct.
	// sizeof(ContractState<T, contractIndex>) == sizeof(T), standard layout, zero-init compatible.
	// Use get() for reads, mut() for writes (marks dirty).
	template <typename T, unsigned int contractIndex>
	struct ContractState {
		static constexpr unsigned int __contract_index = contractIndex;
		const T& get() const { return _data; }
		T& mut() { ::__markContractStateDirty(contractIndex); return _data; }
	private:
		T _data;
	};

	// Letters for defining identity with ID function
	constexpr long long _A = 0;
	constexpr long long _B = 1;
	constexpr long long _C = 2;
	constexpr long long _D = 3;
	constexpr long long _E = 4;
	constexpr long long _F = 5;
	constexpr long long _G = 6;
	constexpr long long _H = 7;
	constexpr long long _I = 8;
	constexpr long long _J = 9;
	constexpr long long _K = 10;
	constexpr long long _L = 11;
	constexpr long long _M = 12;
	constexpr long long _N = 13;
	constexpr long long _O = 14;
	constexpr long long _P = 15;
	constexpr long long _Q = 16;
	constexpr long long _R = 17;
	constexpr long long _S = 18;
	constexpr long long _T = 19;
	constexpr long long _U = 20;
	constexpr long long _V = 21;
	constexpr long long _W = 22;
	constexpr long long _X = 23;
	constexpr long long _Y = 24;
	constexpr long long _Z = 25;

	inline id ID(long long _00, long long _01, long long _02, long long _03, long long _04, long long _05, long long _06, long long _07, long long _08, long long _09,
		long long _10, long long _11, long long _12, long long _13, long long _14, long long _15, long long _16, long long _17, long long _18, long long _19,
		long long _20, long long _21, long long _22, long long _23, long long _24, long long _25, long long _26, long long _27, long long _28, long long _29,
		long long _30, long long _31, long long _32, long long _33, long long _34, long long _35, long long _36, long long _37, long long _38, long long _39,
		long long _40, long long _41, long long _42, long long _43, long long _44, long long _45, long long _46, long long _47, long long _48, long long _49,
		long long _50, long long _51, long long _52, long long _53, long long _54, long long _55)
	{ 
		return _mm256_set_epi64x(((((((((((((((uint64)_55) * 26 + _54) * 26 + _53) * 26 + _52) * 26 + _51) * 26 + _50) * 26 + _49) * 26 + _48) * 26 + _47) * 26 + _46) * 26 + _45) * 26 + _44) * 26 + _43) * 26 + _42, ((((((((((((((uint64)_41) * 26 + _40) * 26 + _39) * 26 + _38) * 26 + _37) * 26 + _36) * 26 + _35) * 26 + _34) * 26 + _33) * 26 + _32) * 26 + _31) * 26 + _30) * 26 + _29) * 26 + _28, ((((((((((((((uint64)_27) * 26 + _26) * 26 + _25) * 26 + _24) * 26 + _23) * 26 + _22) * 26 + _21) * 26 + _20) * 26 + _19) * 26 + _18) * 26 + _17) * 26 + _16) * 26 + _15) * 26 + _14, ((((((((((((((uint64)_13) * 26 + _12) * 26 + _11) * 26 + _10) * 26 + _09) * 26 + _08) * 26 + _07) * 26 + _06) * 26 + _05) * 26 + _04) * 26 + _03) * 26 + _02) * 26 + _01) * 26 + _00); 
	}

#define NUMBER_OF_COMPUTORS 676
#define QUORUM (NUMBER_OF_COMPUTORS * 2 / 3 + 1)

constexpr int JANUARY = 1;
constexpr int FEBRUARY = 2;
constexpr int MARCH = 3;
constexpr int APRIL = 4;
constexpr int MAY = 5;
constexpr int JUNE = 6;
constexpr int JULY = 7;
constexpr int AUGUST = 8;
constexpr int SEPTEMBER = 9;
constexpr int OCTOBER = 10;
constexpr int NOVEMBER = 11;
constexpr int DECEMBER = 12;

constexpr int WEDNESDAY = 0;
constexpr int THURSDAY = 1;
constexpr int FRIDAY = 2;
constexpr int SATURDAY = 3;
constexpr int SUNDAY = 4;
constexpr int MONDAY = 5;
constexpr int TUESDAY = 6;

constexpr unsigned long long X_MULTIPLIER = 1ULL;

// Array of L elements of type T (L must be 2^N)
template <typename T, uint64 L>
struct Array
{
private:
    static_assert(L && !(L & (L - 1)),
        "The capacity of the array must be 2^N."
        );

    T _values[L];

public:
    // Return number of elements in array
    static inline constexpr uint64 capacity()
    {
        return L;
    }

    // Get element of array
    inline const T& get(uint64 index) const
    {
        return _values[index & (L - 1)];
    }

    // Set element of array
    inline void set(uint64 index, const T& value)
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
        for (uint64 i = 0; i < L; ++i)
            _values[i] = value;
    }

    // Set elements in range to passed value
    inline void setRange(uint64 indexBegin, uint64 indexEnd, const T& value)
    {
        for (uint64 i = indexBegin; i < indexEnd; ++i)
            _values[i & (L - 1)] = value;
    }

    // Returns true if all elements of the range equal value (and range is valid).
    inline bool rangeEquals(uint64 indexBegin, uint64 indexEnd, const T& value) const
    {
        if (indexEnd > L || indexBegin > indexEnd)
            return false;
        for (uint64 i = indexBegin; i < indexEnd; ++i)
        {
            if (!(_values[i] == value))
                return false;
        }
        return true;
    }

    // Implement assignment operator to prevent generating call to unavailable memcpy()
    inline Array<T, L>& operator=(const Array<T, L>& other)
    {
        copyMemory(*this, other);
        return *this;
    }

    // Implement copy constructor to prevent generating call to unavailable memcpy()
    inline Array(const Array<T, L>& other)
    {
        copyMemory(*this, other);
    }

    Array() = default;
};

// Array convenience definitions
typedef Array<sint8, 2> sint8_2;
typedef Array<sint8, 4> sint8_4;
typedef Array<sint8, 8> sint8_8;

typedef Array<uint8, 2> uint8_2;
typedef Array<uint8, 4> uint8_4;
typedef Array<uint8, 8> uint8_8;

typedef Array<sint16, 2> sint16_2;
typedef Array<sint16, 4> sint16_4;
typedef Array<sint16, 8> sint16_8;

typedef Array<uint16, 2> uint16_2;
typedef Array<uint16, 4> uint16_4;
typedef Array<uint16, 8> uint16_8;

typedef Array<sint32, 2> sint32_2;
typedef Array<sint32, 4> sint32_4;
typedef Array<sint32, 8> sint32_8;

typedef Array<uint32, 2> uint32_2;
typedef Array<uint32, 4> uint32_4;
typedef Array<uint32, 8> uint32_8;

typedef Array<sint64, 2> sint64_2;
typedef Array<sint64, 4> sint64_4;
typedef Array<sint64, 8> sint64_8;

typedef Array<uint64, 2> uint64_2;
typedef Array<uint64, 4> uint64_4;
typedef Array<uint64, 8> uint64_8;

typedef Array<id, 2> id_2;
typedef Array<id, 8> id_4;
typedef Array<id, 8> id_8;

// Collection of priority queues of elements with type T and total element capacity L.
// Each ID pov (point of view) has an own queue.
template <typename T, uint64 L>
struct Collection
{
private:
    static_assert(L && !(L & (L - 1)),
        "The capacity of the Collection must be 2^N."
        );
    static constexpr sint64 _nEncodedFlags = L > 32 ? 32 : L;

    // Hash map of point of views = element filters, each with one priority queue (or empty)
    struct PoV
    {
        id value;
        uint64 population;
        sint64 headIndex, tailIndex;
        sint64 bstRootIndex;
    } _povs[L];

    // 2 bits per element of _povs: 0b00 = not occupied; 0b01 = occupied; 0b10 = occupied but marked for removal; 0b11 is unused
    // The state "occupied but marked for removal" is needed for finding the index of a pov in the hash map. Setting an entry to
    // "not occupied" in remove() would potentially undo a collision, create a gap, and mess up the entry search.
    uint64 _povOccupationFlags[(L * 2 + 63) / 64];

    // Array of elements (filled sequentially), each belongs to one PoV / priority queue (or is empty)
    // Elements of a POV entry will be stored as a binary search tree (BST); so this structure has some properties related to BST
    // (bstParentIndex, bstLeftIndex, bstRightIndex).
    struct Element
    {
        T value;
        sint64 priority;
        sint64 povIndex;
        sint64 bstParentIndex;
        sint64 bstLeftIndex;
        sint64 bstRightIndex;

        Element& init(const T& value, const sint64& priority, const sint64& povIndex)
        {
            this->value = value;
            this->priority = priority;
            this->povIndex = povIndex;
            this->bstParentIndex = NULL_INDEX;
            this->bstLeftIndex = NULL_INDEX;
            this->bstRightIndex = NULL_INDEX;
            return *this;
        }
    } _elements[L];
    uint64 _population;
    uint64 _markRemovalCounter;

    // Internal reinitialize as empty collection.
    void _softReset();

    // Return index of id pov in hash map _povs, or NULL_INDEX if not found
    sint64 _povIndex(const id& pov) const;

    // Return elementIndex of first element in priority queue of pov,
    // and ignore elements with priority greater than maxPriority
    sint64 _headIndex(const sint64 povIndex, const sint64 maxPriority) const;

    // Return elementIndex of last element in priority queue of pov,
    // and ignore elements with priority less than minPriority
    sint64 _tailIndex(const sint64 povIndex, const sint64 minPriority) const;

    // Return index of parent element to insert a priority
    sint64 _searchElement(const sint64 bstRootIndex,
        const sint64 priority, int* pIterationsCount = nullptr) const;

    // Add element to priority queue, return elementIndex of new element
    sint64 _addPovElement(const sint64 povIndex, const T value, const sint64 priority);

    // Get element indices and store them in an array, return number of elements
    uint64 _getSortedElements(const sint64 rootIdx, sint64* sortedElementIndices) const;

    // Fill a sint64_4 vector with specified values
    inline void _set(sint64_4& vec, sint64 v0, sint64 v1, sint64 v2, sint64 v3) const;

    // Rebuild pov's elements indexing as balanced BST
    sint64 _rebuild(sint64 rootIdx);

    // Return most left element index
    sint64 _getMostLeft(sint64 elementIdx) const;

    // Return most right element index
    sint64 _getMostRight(sint64 elementIdx) const;

    // Return elementIndex of previous element in priority queue (or NULL_INDEX if this is the last element).
    sint64 _previousElementIndex(sint64 elementIdx) const;

    // Return elementIndex of next element in priority queue (or NULL_INDEX if this is the last element).
    sint64 _nextElementIndex(sint64 elementIdx) const;

    // Update parent of the current element into parent of the new element, return true if exists parent
    inline bool _updateParent(const sint64 elementIdx, const sint64 newElementIdx);

    // Move the current element into new position
    void _moveElement(const sint64 srcIdx, const sint64 dstIdx);

    // Read and encode 32 POV occupation flags, return a 64bits number presents 32 occupation flags
    uint64 _getEncodedPovOccupationFlags(const uint64* povOccupationFlags, const sint64 povIndex) const;;

public:
    // Add element to priority queue of ID pov, return elementIndex of new element
    sint64 add(const id& pov, T element, sint64 priority);

    // Return maximum number of elements that may be stored.
    static constexpr uint64 capacity()
    {
        return L;
    }

    // Check if cleanup is needed based on the removal threshold, without modifying the collection.
    bool needsCleanup(uint64 removalThresholdPercent = 50) const;

    // Call cleanup() if more than the given percent of pov slots are marked for removal.
    void cleanupIfNeeded(uint64 removalThresholdPercent = 50);

    // Remove all povs marked for removal, this is a very expensive operation, but it improves lookup performance
    // if remove has been called often. Content is reordered, so prior indices are invalidated.
    void cleanup();

    // Return element value at elementIndex.
    inline T element(sint64 elementIndex) const;

    // Return elementIndex of first element in priority queue of pov (or NULL_INDEX if pov is unknown).
    sint64 headIndex(const id& pov) const;

    // Return elementIndex of first element with priority <= maxPriority in priority queue of pov (or NULL_INDEX if pov is unknown).
    sint64 headIndex(const id& pov, sint64 maxPriority) const;

    // Return elementIndex of next element in priority queue (or NULL_INDEX if this is the last element).
    sint64 nextElementIndex(sint64 elementIndex) const;

    // Return overall number of elements.
    inline uint64 population() const;

    // Return number of elements of specific PoV.
    uint64 population(const id& pov) const;

    // Return point of view elementIndex belongs to (or 0 id if unused).
    id pov(sint64 elementIndex) const;

    // Return elementIndex of previous element in priority queue (or NULL_INDEX if this is the last element).
    sint64 prevElementIndex(sint64 elementIndex) const;

    // Return priority of elementIndex (or 0 id if unused).
    sint64 priority(sint64 elementIndex) const;

    // Remove element and mark its pov for removal, if the last element.
    // Returns element index of next element in priority queue (the one following elementIdx).
    // Element indices obtained before this call are invalidated, because at least one element is moved.
    sint64 remove(sint64 elementIdx);

    // Replace *existing* element, do nothing otherwise.
    // - The element exists: replace its value.
    // - The index is out of bounds: no action is taken.
    void replace(sint64 oldElementIndex, const T& newElement);

    // Reinitialize as empty collection.
    void reset();

    // Return elementIndex of last element in priority queue of pov (or NULL_INDEX if pov is unknown).
    sint64 tailIndex(const id& pov) const;

    // Return elementIndex of last element with priority >= minPriority in priority queue of pov (or NULL_INDEX if pov is unknown).
    sint64 tailIndex(const id& pov, sint64 minPriority) const;
};

template <typename T, uint64 L>
void Collection<T, L>::_softReset()
{
    setMem(_povs, sizeof(_povs), 0);
    setMem(_povOccupationFlags, sizeof(_povOccupationFlags), 0);
    _population = 0;
    _markRemovalCounter = 0;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_povIndex(const id& pov) const
{
    sint64 povIndex = pov.u64._0 & (L - 1);
    for (sint64 counter = 0; counter < L; counter += 32)
    {
        uint64 flags = _getEncodedPovOccupationFlags(_povOccupationFlags, povIndex);
        for (auto i = 0; i < _nEncodedFlags; i++, flags >>= 2)
        {
            switch (flags & 3ULL)
            {
            case 0:
                return NULL_INDEX;
            case 1:
                if (_povs[povIndex].value == pov)
                {
                    return povIndex;
                }
                break;
            }
            povIndex = (povIndex + 1) & (L - 1);
        }
    }
    return NULL_INDEX;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_headIndex(const sint64 povIndex, const sint64 maxPriority) const
{
    // with current code path, pov is not empty here
    const auto& pov = _povs[povIndex];

    // quick check head/tail
    if (_elements[pov.headIndex].priority <= maxPriority)
    {
        return pov.headIndex;
    }
    if (_elements[pov.tailIndex].priority > maxPriority)
    {
        return NULL_INDEX;
    }

    // here, head's priority > maxPriority >= tail's priority
    // => always found a valid element

    // search index of parent element
    // - always found parent element because pov is not empty
    sint64 idx = _searchElement(pov.bstRootIndex, maxPriority);
    if (_elements[idx].priority > maxPriority)
    {
        // forward iterating until meet element having priority <= maxPriority
        while (true)
        {
            idx = _nextElementIndex(idx);
            if (_elements[idx].priority <= maxPriority)
            {
                break;
            }
        }
        return idx;
    }

    // backward iterating until meet element having priority > maxPriority
    while (true)
    {
        sint64 prevIdx = _previousElementIndex(idx);
        if (prevIdx == NULL_INDEX || _elements[prevIdx].priority > maxPriority)
        {
            break;
        }
        idx = prevIdx;
    }
    return idx;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_tailIndex(const sint64 povIndex, const sint64 minPriority) const
{
    // with current code path, pov is not empty here
    const auto& pov = _povs[povIndex];

    // quick check head/tail
    if (_elements[pov.headIndex].priority < minPriority)
    {
        return NULL_INDEX;
    }
    if (_elements[pov.tailIndex].priority >= minPriority)
    {
        return pov.tailIndex;
    }

    // here, head's priority >= minPriority > tail's priority
    // => always found a valid element

    // search index of parent element
    // - always found parent element because pov is not empty
    sint64 idx = _searchElement(pov.bstRootIndex, minPriority);

    if (_elements[idx].priority >= minPriority)
    {
        // forward iterating until meet element having priority < minPriority
        while (true)
        {
            sint64 nextIdx = _nextElementIndex(idx);
            if (nextIdx == NULL_INDEX || _elements[nextIdx].priority < minPriority)
            {
                break;
            }
            idx = nextIdx;
        }
        return idx;
    }

    // backward iterating to meet element having priority >= minPriority
    while (true)
    {
        idx = _previousElementIndex(idx);
        if (_elements[idx].priority >= minPriority)
        {
            break;
        }
    }
    return idx;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_searchElement(const sint64 bstRootIndex,
    const sint64 priority, int* pIterationsCount) const
{
    sint64 idx = bstRootIndex;
    while (idx != NULL_INDEX)
    {
        if (pIterationsCount)
        {
            *pIterationsCount += 1;
        }
        auto& curElement = _elements[idx];
        if (curElement.priority >= priority)
        {
            if (curElement.bstRightIndex != NULL_INDEX)
            {
                idx = curElement.bstRightIndex;
            }
            else
            {
                return idx;
            }
        }
        else
        {
            if (curElement.bstLeftIndex != NULL_INDEX)
            {
                idx = curElement.bstLeftIndex;
            }
            else
            {
                return idx;
            }
        }
    }
    return NULL_INDEX;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_addPovElement(const sint64 povIndex, const T value, const sint64 priority)
{
    const sint64 newElementIdx = _population++;
    auto& newElement = _elements[newElementIdx].init(value, priority, povIndex);
    auto& pov = _povs[povIndex];

    if (pov.population == 0)
    {
        pov.population = 1;
        pov.headIndex = newElementIdx;
        pov.tailIndex = newElementIdx;
        pov.bstRootIndex = newElementIdx;
    }
    else
    {
        int iterations_count = 0;
        sint64 parentIdx = _searchElement(pov.bstRootIndex, priority, &iterations_count);
        if (_elements[parentIdx].priority >= priority)
        {
            _elements[parentIdx].bstRightIndex = newElementIdx;
        }
        else
        {
            _elements[parentIdx].bstLeftIndex = newElementIdx;
        }
        newElement.bstParentIndex = parentIdx;
        pov.population++;


        if (_elements[pov.headIndex].priority < priority)
        {
            pov.headIndex = newElementIdx;
        }
        else if (_elements[pov.tailIndex].priority >= priority)
        {
            pov.tailIndex = newElementIdx;
        }
        if (pov.population > 32 && iterations_count > pov.population / 4)
        {
            // make balanced binary search tree to get better performance
            pov.bstRootIndex = _rebuild(pov.bstRootIndex);
        }
    }
    return newElementIdx;
}

template <typename T, uint64 L>
uint64 Collection<T, L>::_getSortedElements(const sint64 rootIdx, sint64* sortedElementIndices) const
{
    uint64 count = 0;
    sint64 elementIdx = rootIdx;
    sint64 lastElementIdx = NULL_INDEX;
    while (elementIdx != NULL_INDEX)
    {
        if (lastElementIdx == _elements[elementIdx].bstParentIndex)
        {
            if (_elements[elementIdx].bstLeftIndex != NULL_INDEX)
            {
                lastElementIdx = elementIdx;
                elementIdx = _elements[elementIdx].bstLeftIndex;
                continue;
            }
            lastElementIdx = NULL_INDEX;
        }
        if (lastElementIdx == _elements[elementIdx].bstLeftIndex)
        {
            sortedElementIndices[count++] = elementIdx;

            if (_elements[elementIdx].bstRightIndex != NULL_INDEX)
            {
                lastElementIdx = elementIdx;
                elementIdx = _elements[elementIdx].bstRightIndex;
                continue;
            }
            lastElementIdx = NULL_INDEX;
        }
        if (lastElementIdx == _elements[elementIdx].bstRightIndex)
        {
            lastElementIdx = elementIdx;
            elementIdx = _elements[elementIdx].bstParentIndex;
        }
    }
    return count;
}

template <typename T, uint64 L>
inline void Collection<T, L>::_set(sint64_4& vec, sint64 v0, sint64 v1, sint64 v2, sint64 v3) const
{
    vec.set(0, v0);
    vec.set(1, v1);
    vec.set(2, v2);
    vec.set(3, v3);
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_rebuild(sint64 rootIdx)
{
    __ScopedScratchpad scratchpad(sizeof(*this), /*initZero=*/false);
    auto* sortedElementIndices = reinterpret_cast<sint64*>(scratchpad.ptr);
    if (sortedElementIndices == NULL)
    {
        return rootIdx;
    }
    sint64 n = _getSortedElements(rootIdx, sortedElementIndices);
    if (!n)
    {
        return rootIdx;
    }
    // initialize root
    sint64 mid = n / 2;
    rootIdx = sortedElementIndices[mid];
    _elements[rootIdx].bstParentIndex = NULL_INDEX;
    _elements[rootIdx].bstLeftIndex = NULL_INDEX;
    _elements[rootIdx].bstRightIndex = NULL_INDEX;
    // initialize queue
    auto* queue = reinterpret_cast<sint64_4*>(sortedElementIndices + ((n + 3) / 4) * 4);
    sint64 dequeueIdx = 0;
    sint64 enqueueIdx = 0;
    sint64 queueSize = 0;
    // push left and right ranges to the queue
    if (mid > 0)
    {
        _set(queue[enqueueIdx], rootIdx, 0, mid - 1, mid);
        enqueueIdx = (enqueueIdx + 1) & (L - 1);
        queueSize++;
    }
    if (mid + 1 < n)
    {
        _set(queue[enqueueIdx], rootIdx, mid + 1, n - 1, mid);
        enqueueIdx = (enqueueIdx + 1) & (L - 1);
        queueSize++;
    }
    while (queueSize > 0)
    {
        // get the front element from the queue
        auto curRange = queue[dequeueIdx];
        dequeueIdx = (dequeueIdx + 1) & (L - 1);
        queueSize--;

        // get the parent node and range
        const auto parentElementIdx = curRange.get(0);
        const auto left = curRange.get(1);
        const auto right = curRange.get(2);

        if (left <= right) // if there are elements to process
        {
            mid = (left + right) / 2;
            const auto elementIdx = sortedElementIndices[mid];
            _elements[elementIdx].bstParentIndex = parentElementIdx;
            _elements[elementIdx].bstLeftIndex = NULL_INDEX;
            _elements[elementIdx].bstRightIndex = NULL_INDEX;

            // set the child node for the parent node
            if (mid < curRange.get(3))
            {
                _elements[parentElementIdx].bstLeftIndex = elementIdx;
            }
            else
            {
                _elements[parentElementIdx].bstRightIndex = elementIdx;
            }

            // push left and right ranges to the queue
            if (mid > left)
            {
                _set(queue[enqueueIdx], elementIdx, left, mid - 1, mid);
                enqueueIdx = (enqueueIdx + 1) & (L - 1);
                queueSize++;
            }
            if (mid < right)
            {
                _set(queue[enqueueIdx], elementIdx, mid + 1, right, mid);
                enqueueIdx = (enqueueIdx + 1) & (L - 1);
                queueSize++;
            }
        }
    }

    return rootIdx;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_getMostLeft(sint64 elementIdx) const
{
    while (_elements[elementIdx].bstLeftIndex != NULL_INDEX)
    {
        elementIdx = _elements[elementIdx].bstLeftIndex;
    }
    return elementIdx;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_getMostRight(sint64 elementIdx) const
{
    while (_elements[elementIdx].bstRightIndex != NULL_INDEX)
    {
        elementIdx = _elements[elementIdx].bstRightIndex;
    }
    return elementIdx;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_previousElementIndex(sint64 elementIdx) const
{
    elementIdx &= (L - 1);
    if (uint64(elementIdx) < _population)
    {
        if (_elements[elementIdx].bstLeftIndex != NULL_INDEX)
        {
            return _getMostRight(_elements[elementIdx].bstLeftIndex);
        }
        else if (_elements[elementIdx].bstParentIndex != NULL_INDEX)
        {
            auto parentIdx = _elements[elementIdx].bstParentIndex;
            if (_elements[parentIdx].bstRightIndex == elementIdx)
            {
                return parentIdx;
            }
            if (_elements[parentIdx].bstLeftIndex == elementIdx)
            {
                while (parentIdx != NULL_INDEX && _elements[parentIdx].bstLeftIndex == elementIdx)
                {
                    elementIdx = parentIdx;
                    parentIdx = _elements[elementIdx].bstParentIndex;
                }
                return parentIdx;
            }
        }
    }
    return NULL_INDEX;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::_nextElementIndex(sint64 elementIdx) const
{
    elementIdx &= (L - 1);
    if (uint64(elementIdx) < _population)
    {
        if (_elements[elementIdx].bstRightIndex != NULL_INDEX)
        {
            return _getMostLeft(_elements[elementIdx].bstRightIndex);
        }
        else if (_elements[elementIdx].bstParentIndex != NULL_INDEX)
        {
            auto parentIdx = _elements[elementIdx].bstParentIndex;
            if (_elements[parentIdx].bstLeftIndex == elementIdx)
            {
                return parentIdx;
            }
            if (_elements[parentIdx].bstRightIndex == elementIdx)
            {
                while (parentIdx != NULL_INDEX && _elements[parentIdx].bstRightIndex == elementIdx)
                {
                    elementIdx = parentIdx;
                    parentIdx = _elements[elementIdx].bstParentIndex;
                }
                return parentIdx;
            }
        }
    }
    return NULL_INDEX;
}

template <typename T, uint64 L>
bool Collection<T, L>::_updateParent(const sint64 elementIdx, const sint64 newElementIdx)
{
    if (elementIdx != NULL_INDEX)
    {
        auto& curElement = _elements[elementIdx];
        if (curElement.bstParentIndex != NULL_INDEX)
        {
            auto& parentElement = _elements[curElement.bstParentIndex];
            if (parentElement.bstRightIndex == elementIdx)
            {
                parentElement.bstRightIndex = newElementIdx;
            }
            else
            {
                parentElement.bstLeftIndex = newElementIdx;
            }
            if (newElementIdx != NULL_INDEX)
            {
                _elements[newElementIdx].bstParentIndex = curElement.bstParentIndex;
            }
            return true;
        }
    }
    return false;
}

template <typename T, uint64 L>
void Collection<T, L>::_moveElement(const sint64 srcIdx, const sint64 dstIdx)
{
    copyMem(&_elements[dstIdx], &_elements[srcIdx], sizeof(_elements[0]));

    const auto povIndex = _elements[dstIdx].povIndex;
    auto& pov = _povs[povIndex];
    if (pov.bstRootIndex == srcIdx)
    {
        pov.bstRootIndex = dstIdx;
    }
    if (pov.headIndex == srcIdx)
    {
        pov.headIndex = dstIdx;
    }
    if (pov.tailIndex == srcIdx)
    {
        pov.tailIndex = dstIdx;
    }

    auto& element = _elements[dstIdx];
    if (element.bstLeftIndex != NULL_INDEX)
    {
        _elements[element.bstLeftIndex].bstParentIndex = dstIdx;
    }
    if (element.bstRightIndex != NULL_INDEX)
    {
        _elements[element.bstRightIndex].bstParentIndex = dstIdx;
    }
    if (element.bstParentIndex != NULL_INDEX)
    {
        auto& parentElement = _elements[element.bstParentIndex];
        if (parentElement.bstLeftIndex == srcIdx)
        {
            parentElement.bstLeftIndex = dstIdx;
        }
        else
        {
            parentElement.bstRightIndex = dstIdx;
        }
    }
}

template <typename T, uint64 L>
uint64 Collection<T, L>::_getEncodedPovOccupationFlags(const uint64* povOccupationFlags, const sint64 povIndex) const
{
    const sint64 offset = (povIndex & 31) << 1;
    uint64 flags = povOccupationFlags[povIndex >> 5] >> offset;
    if (offset > 0)
    {
        flags |= povOccupationFlags[((povIndex + 32) & (L - 1)) >> 5] << (2 * _nEncodedFlags - offset);
    }
    return flags;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::add(const id& pov, T element, sint64 priority)
{
    if (_population < capacity())
    {
        // search in pov hash map
        sint64 markedForRemovalIndexForReuse = NULL_INDEX;
        sint64 povIndex = pov.u64._0 & (L - 1);
        for (sint64 counter = 0; counter < L; counter += 32)
        {
            uint64 flags = _getEncodedPovOccupationFlags(_povOccupationFlags, povIndex);
            for (auto i = 0; i < _nEncodedFlags; i++, flags >>= 2)
            {
                switch (flags & 3ULL)
                {
                case 0:
                    // empty hash map entry -> pov isn't in map yet
                    // If we have already seen an entry marked for removal, reuse this slot because it is closer to the hash index
                    if (markedForRemovalIndexForReuse != NULL_INDEX)
                        goto reuse_slot;
                    // ... otherwise mark as occupied and init new priority queue with 1 element
                    _povOccupationFlags[povIndex >> 5] |= (1ULL << ((povIndex & 31) << 1));
                    _povs[povIndex].value = pov;
                    return _addPovElement(povIndex, element, priority);
                case 1:
                    if (_povs[povIndex].value == pov)
                    {
                        // found pov entry -> insert element in priority queue of pov
                        return _addPovElement(povIndex, element, priority);
                    }
                    break;
                case 2:
                    // marked for removal -> reuse slot (first slot we see) later if we are sure that key isn't in the set
                    if (markedForRemovalIndexForReuse == NULL_INDEX)
                        markedForRemovalIndexForReuse = povIndex;
                    break;
                }
                povIndex = (povIndex + 1) & (L - 1);
            }
        }

        if (markedForRemovalIndexForReuse != NULL_INDEX)
        {
        reuse_slot:
            // Reuse slot marked for removal: put pov key here and set flags from 2 to 1.
            // But don't decrement _markRemovalCounter, because it is used to check if cleanup() is needed.
            // Without cleanup, we don't get new unoccupied slots and at least lookup of povs that aren't contained in the set
            // stays slow.
            povIndex = markedForRemovalIndexForReuse;
            _povOccupationFlags[povIndex >> 5] ^= (3ULL << ((povIndex & 31) << 1));
            _povs[povIndex].value = pov;
            return _addPovElement(povIndex, element, priority);
        }
    }
    return NULL_INDEX;
}

template <typename T, uint64 L>
bool Collection<T, L>::needsCleanup(uint64 removalThresholdPercent) const
{
    return _markRemovalCounter > (removalThresholdPercent * L / 100);
}

template <typename T, uint64 L>
void Collection<T, L>::cleanupIfNeeded(uint64 removalThresholdPercent)
{
    if (_markRemovalCounter > (removalThresholdPercent * L / 100))
    {
        cleanup();
    }
}

template <typename T, uint64 L>
void Collection<T, L>::cleanup()
{
    // _povs gets occupied over time with entries of type 3 which means they are marked for cleanup.
    // Once cleanup is called it's necessary to remove all these type 3 entries by reconstructing a fresh Collection residing in scratchpad buffer.
    // The _elements array is not reorganized by the cleanup (only references to _povs are updated).
    // Cleanup() called for a Collection having only type 3 entries in _povs must give the result equal to reset() memory content wise.

    // Quick check to cleanup
    if (!_markRemovalCounter)
    {
        return;
    }

    // Speedup case of empty Collection but existed marked for removal povs
    if (!population())
    {
        _softReset();
        return;
    }

    // Init buffers. Besides the rebuilt pov tables we also need traversal stack
    // space for walking a pov's BST while updating element.povIndex.
    const uint64 stackBytes = _population * sizeof(sint64);
    __ScopedScratchpad scratchpad(sizeof(_povs) + sizeof(_povOccupationFlags) + stackBytes, /*initZero=*/true);
    ASSERT(scratchpad.ptr);
    auto* _povsBuffer = reinterpret_cast<PoV*>(scratchpad.ptr);
    auto* _povOccupationFlagsBuffer = reinterpret_cast<uint64*>(_povsBuffer + L);
    auto* _stackBuffer = reinterpret_cast<sint64*>(
        _povOccupationFlagsBuffer + sizeof(_povOccupationFlags) / sizeof(_povOccupationFlags[0]));
    uint64 newPopulation = 0;

    // Go through pov hash map. For each pov that is occupied but not marked for removal, insert pov in new Collection's pov buffers and
    // update povIndex in elements belonging to pov.
    constexpr uint64 oldPovIndexGroupCount = (L >> 5) ? (L >> 5) : 1;
    for (sint64 oldPovIndexGroup = 0; oldPovIndexGroup < oldPovIndexGroupCount; oldPovIndexGroup++)
    {
        const uint64 flags = _povOccupationFlags[oldPovIndexGroup];
        uint64 maskBits = (0xAAAAAAAAAAAAAAAA & (flags << 1));
        maskBits &= maskBits ^ (flags & 0xAAAAAAAAAAAAAAAA);
        sint64 oldPovIndexOffset = _tzcnt_u64(maskBits) & 0xFE;
        const sint64 oldPovIndexOffsetEnd = 64 - (_lzcnt_u64(maskBits) & 0xFE);
        for (maskBits >>= oldPovIndexOffset;
            oldPovIndexOffset < oldPovIndexOffsetEnd; oldPovIndexOffset += 2, maskBits >>= 2)
        {
            // Only add pov to new Collection that are occupied and not marked for removal
            if (maskBits & 3ULL)
            {
                // find empty position in new pov hash map
                const sint64 oldPovIndex = (oldPovIndexGroup << 5) + (oldPovIndexOffset >> 1);
                sint64 newPovIndex = _povs[oldPovIndex].value.u64._0 & (L - 1);
                for (sint64 counter = 0; counter < L; counter += 32)
                {
                    QPI::uint64 newFlags = _getEncodedPovOccupationFlags(_povOccupationFlagsBuffer, newPovIndex);
                    for (sint64 i = 0; i < _nEncodedFlags; i++, newFlags >>= 2)
                    {
                        if ((newFlags & 3ULL) == 0)
                        {
                            newPovIndex = (newPovIndex + i) & (L - 1);
                            goto foundEmptyPosition;
                        }
                    }
                    newPovIndex = (newPovIndex + _nEncodedFlags) & (L - 1);
                }
#ifdef NO_UEFI
                // should never be reached, because old and new map have same capacity (there should always be an empty slot)
                goto cleanupBug;
#endif

            foundEmptyPosition:
                // occupy empty pov hash map entry
                _povOccupationFlagsBuffer[newPovIndex >> 5] |= (1ULL << ((newPovIndex & 31) << 1));
                copyMem(&_povsBuffer[newPovIndex], &_povs[oldPovIndex], sizeof(PoV));

                // update newPovIndex for elements
                if (newPovIndex != oldPovIndex)
                {
                    sint64 stackSize = 0;
                    _stackBuffer[stackSize++] = _povsBuffer[newPovIndex].bstRootIndex;
                    while (stackSize > 0)
                    {
                        auto& element = _elements[_stackBuffer[--stackSize]];
                        element.povIndex = newPovIndex;
                        if (element.bstLeftIndex != NULL_INDEX)
                        {
                            _stackBuffer[stackSize++] = element.bstLeftIndex;
                        }
                        if (element.bstRightIndex != NULL_INDEX)
                        {
                            _stackBuffer[stackSize++] = element.bstRightIndex;
                        }
                    }
                }

                // check if we are done
                newPopulation += _povs[oldPovIndex].population;
                if (newPopulation == _population)
                {
                    // povs of all elements have been transferred -> overwrite old pov arrays with new pov arrays
                    copyMem(_povs, _povsBuffer, sizeof(_povs));
                    copyMem(_povOccupationFlags, _povOccupationFlagsBuffer, sizeof(_povOccupationFlags));
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

template <typename T, uint64 L>
inline T Collection<T, L>::element(sint64 elementIndex) const
{
    return _elements[elementIndex & (L - 1)].value;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::headIndex(const id& pov) const
{
    const sint64 povIndex = _povIndex(pov);

    return povIndex < 0 ? NULL_INDEX : _povs[povIndex].headIndex;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::headIndex(const id& pov, sint64 maxPriority) const
{
    const sint64 povIndex = _povIndex(pov);
    if (povIndex < 0)
    {
        return NULL_INDEX;
    }

    return _headIndex(povIndex, maxPriority);
}

template <typename T, uint64 L>
sint64 Collection<T, L>::nextElementIndex(sint64 elementIndex) const
{
    return _nextElementIndex(elementIndex);
}

template <typename T, uint64 L>
inline uint64 Collection<T, L>::population() const
{
    return _population;
}

template <typename T, uint64 L>
uint64 Collection<T, L>::population(const id& pov) const
{
    const sint64 povIndex = _povIndex(pov);

    return povIndex < 0 ? 0 : _povs[povIndex].population;
}

template <typename T, uint64 L>
id Collection<T, L>::pov(sint64 elementIndex) const
{
    return _povs[_elements[elementIndex & (L - 1)].povIndex].value;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::prevElementIndex(sint64 elementIndex) const
{
    return _previousElementIndex(elementIndex);
}

template <typename T, uint64 L>
sint64 Collection<T, L>::priority(sint64 elementIndex) const
{
    return _elements[elementIndex & (L - 1)].priority;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::remove(sint64 elementIdx)
{
    sint64 nextElementIdxOfRemoved = NULL_INDEX;
    elementIdx &= (L - 1);
    if (uint64(elementIdx) < _population)
    {
        auto deleteElementIdx = elementIdx;
        const auto povIndex = _elements[elementIdx].povIndex;
        auto& pov = _povs[povIndex];
        if (pov.population > 1)
        {
            auto& rootIdx = pov.bstRootIndex;
            auto& curElement = _elements[elementIdx];

            nextElementIdxOfRemoved = _nextElementIndex(elementIdx);

            if (curElement.bstRightIndex != NULL_INDEX &&
                curElement.bstLeftIndex != NULL_INDEX)
            {
                // it contains both left and right child
                // -> move next element in priority queue to curElement, delete next element
                const auto tmpIdx = nextElementIdxOfRemoved;
                if (tmpIdx == pov.tailIndex)
                {
                    pov.tailIndex = _previousElementIndex(tmpIdx);
                }
                const auto rightTmpIndex = _elements[tmpIdx].bstRightIndex;
                if (tmpIdx == curElement.bstRightIndex)
                {
                    curElement.bstRightIndex = rightTmpIndex;
                    if (rightTmpIndex != NULL_INDEX)
                    {
                        _elements[rightTmpIndex].bstParentIndex = elementIdx;
                    }
                }
                else
                {
                    _elements[_elements[tmpIdx].bstParentIndex].bstLeftIndex = rightTmpIndex;
                    if (rightTmpIndex != NULL_INDEX)
                    {
                        _elements[rightTmpIndex].bstParentIndex = _elements[tmpIdx].bstParentIndex;
                    }
                }
                copyMem(&curElement.value, &_elements[tmpIdx].value, sizeof(T));
                curElement.priority = _elements[tmpIdx].priority;
                nextElementIdxOfRemoved = elementIdx;

                deleteElementIdx = tmpIdx;
            }
            else if (curElement.bstRightIndex != NULL_INDEX)
            {
                // contains only right child
                if (elementIdx == pov.headIndex)
                {
                    pov.headIndex = nextElementIdxOfRemoved;
                }
                if (!_updateParent(elementIdx, curElement.bstRightIndex))
                {
                    rootIdx = curElement.bstRightIndex;
                    _elements[rootIdx].bstParentIndex = NULL_INDEX;
                }
            }
            else if (curElement.bstLeftIndex != NULL_INDEX)
            {
                // contains only left child
                if (elementIdx == pov.tailIndex)
                {
                    pov.tailIndex = _previousElementIndex(elementIdx);
                }
                if (!_updateParent(elementIdx, curElement.bstLeftIndex))
                {
                    rootIdx = curElement.bstLeftIndex;
                    _elements[rootIdx].bstParentIndex = NULL_INDEX;
                }
            }
            else // it's a leaf node
            {
                if (elementIdx == pov.headIndex)
                {
                    pov.headIndex = nextElementIdxOfRemoved;
                }
                else if (elementIdx == pov.tailIndex)
                {
                    pov.tailIndex = _previousElementIndex(elementIdx);
                }
                _updateParent(elementIdx, NULL_INDEX);
            }
            --pov.population;
        }
        else
        {
            pov.population = 0;
            _markRemovalCounter++;
            _povOccupationFlags[povIndex >> 5] ^= (3ULL << ((povIndex & 31) << 1));
        }

        if (--_population && deleteElementIdx != _population)
        {
            // Move last element to fill new gap in array
            _moveElement(_population, deleteElementIdx);
            if (nextElementIdxOfRemoved == _population)
                nextElementIdxOfRemoved = deleteElementIdx;
        }

        const bool CLEAR_UNUSED_ELEMENT = true;
        if (CLEAR_UNUSED_ELEMENT)
        {
            setMem(&_elements[_population], sizeof(Element), 0);
        }
    }

    return nextElementIdxOfRemoved;
}

template <typename T, uint64 L>
void Collection<T, L>::replace(sint64 oldElementIndex, const T& newElement)
{
    if (uint64(oldElementIndex) < _population)
    {
        _elements[oldElementIndex].value = newElement;
    }
}

inline void setMem(void* buffer, unsigned long long size, unsigned char value)
{
    memset(buffer, value, size);
}

template <typename T, uint64 L>
void Collection<T, L>::reset()
{
    setMem(this, sizeof(*this), 0);
}

template <typename T, uint64 L>
sint64 Collection<T, L>::tailIndex(const id& pov) const
{
    const sint64 povIndex = _povIndex(pov);

    return povIndex < 0 ? NULL_INDEX : _povs[povIndex].tailIndex;
}

template <typename T, uint64 L>
sint64 Collection<T, L>::tailIndex(const id& pov, sint64 minPriority) const
{
    const sint64 povIndex = _povIndex(pov);
    if (povIndex < 0)
    {
        return NULL_INDEX;
    }

    return _tailIndex(povIndex, minPriority);
}

// FIXED CONSTANTS (old on-chain layout)
constexpr uint64 QSWAP_OLD_INITIAL_MAX_POOL = 16384;
constexpr uint64 QSWAP_OLD_MAX_POOL = QSWAP_OLD_INITIAL_MAX_POOL * X_MULTIPLIER;

// FIXED CONSTANTS (new contract layout)
constexpr uint64 QSWAP_NEW_INITIAL_MAX_POOL = 8192;
constexpr uint64 QSWAP_NEW_MAX_POOL = QSWAP_NEW_INITIAL_MAX_POOL * X_MULTIPLIER;

constexpr uint64 QSWAP_MAX_USER_PER_POOL = 256;
constexpr sint64 QSWAP_MIN_LIQUIDITY = 1000;
constexpr uint32 QSWAP_SWAP_FEE_BASE = 10000;
constexpr uint32 QSWAP_FEE_BASE_100 = 100;

struct uint128
{
    uint64 low = 0;
    uint64 high = 0;
};

// On-disk layout before migration (no LP fee accumulator on pools).
struct OldPoolBasicState
{
    id poolID;
    sint64 reservedQuAmount;
    sint64 reservedAssetAmount;
    sint64 totalLiquidity;
};

struct PoolBasicState
{
    id poolID;
    sint64 reservedQuAmount;
    sint64 reservedAssetAmount;
    sint64 totalLiquidity;
    uint128 accFeePerLPX64;
};

struct LiquidityInfo
{
    sint64 liquidity;
    uint128 feeDebtX64;
    uint64 accumulatedFee;
};

// Old QSWAP stored entity + liquidity per Collection element (PoV = poolID).
struct OldLiquidityInfo
{
    id entity;
    sint64 liquidity;
};

// Old state variables read from the contract file
uint32 old_swapFeeRate;           // e.g. 30: 0.3% (base: 10_000)
uint32 old_investRewardsFeeRate;  // 3: 3% of swap fees to Invest & Rewards (base: 100)
uint32 old_shareholderFeeRate;    // 27: 27% of swap fees to SC shareholders (base: 100)
uint32 old_poolCreationFeeRate;   // e.g. 10: 10% (base: 100)

id old_investRewardsId;
uint64 old_investRewardsEarnedFee;
uint64 old_investRewardsDistributedAmount;

uint64 old_shareholderEarnedFee;
uint64 old_shareholderDistributedAmount;

Array<OldPoolBasicState, QSWAP_OLD_MAX_POOL> old_mPoolBasicStates;
Collection<OldLiquidityInfo, QSWAP_OLD_MAX_POOL * QSWAP_MAX_USER_PER_POOL> old_mLiquidities;

uint32 old_qxFeeRate;             // 5: 5% of swap fees to QX (base: 100)
uint32 old_burnFeeRate;           // 1: 1% of swap fees burned (base: 100)

uint64 old_qxEarnedFee;
uint64 old_qxDistributedAmount;

uint64 old_burnEarnedFee;         // Total burn fees collected (to be burned in END_TICK)
uint64 old_burnedAmount;          // Total amount actually burned

uint32 old_cachedIssuanceFee;
uint32 old_cachedTransferFee;

// Migrated state (file scope: Collection is hundreds of MB — must not live on the stack).
Array<PoolBasicState, QSWAP_NEW_MAX_POOL> new_mPoolBasicStates;
Collection<LiquidityInfo, QSWAP_NEW_MAX_POOL * QSWAP_MAX_USER_PER_POOL> new_mLiquidities;

// Binary state I/O (field order must match QSWAP::StateData on disk).
#define READ_STATE(stream, value) \
    do { \
        (stream).read(reinterpret_cast<char*>(&(value)), sizeof(value)); \
        if (!(stream) || static_cast<std::size_t>((stream).gcount()) != sizeof(value)) { \
            throw std::runtime_error("Failed to read from contract state file."); \
        } \
    } while (0)

#define WRITE_STATE(stream, value) \
    do { \
        (stream).write(reinterpret_cast<const char*>(&(value)), sizeof(value)); \
        if (!(stream)) { \
            throw std::runtime_error("Failed to write to contract state file."); \
        } \
    } while (0)

// Read old state from the contract file (field order matches QSWAP::StateData)
void readOldState(const std::string& filename)
{
    std::ifstream infile(filename, std::ios::binary);
    if (!infile)
    {
        throw std::runtime_error("Failed to open the old state file: " + filename);
    }

    READ_STATE(infile, old_swapFeeRate);
    READ_STATE(infile, old_investRewardsFeeRate);
    READ_STATE(infile, old_shareholderFeeRate);
    READ_STATE(infile, old_poolCreationFeeRate);

    READ_STATE(infile, old_investRewardsId);
    READ_STATE(infile, old_investRewardsEarnedFee);
    READ_STATE(infile, old_investRewardsDistributedAmount);

    READ_STATE(infile, old_shareholderEarnedFee);
    READ_STATE(infile, old_shareholderDistributedAmount);

    READ_STATE(infile, old_mPoolBasicStates);
    READ_STATE(infile, old_mLiquidities);

    READ_STATE(infile, old_qxFeeRate);
    READ_STATE(infile, old_burnFeeRate);

    READ_STATE(infile, old_qxEarnedFee);
    READ_STATE(infile, old_qxDistributedAmount);

    READ_STATE(infile, old_burnEarnedFee);
    READ_STATE(infile, old_burnedAmount);

    READ_STATE(infile, old_cachedIssuanceFee);
    READ_STATE(infile, old_cachedTransferFee);

    infile.close();
}

static bool isZeroId(const id& value)
{
    return value == id::zero();
}

static void writeCsvEscaped(std::ostream& out, const std::string& field)
{
    bool needQuotes = false;
    for (const char c : field)
    {
        if (c == ',' || c == '"' || c == '\n' || c == '\r')
        {
            needQuotes = true;
            break;
        }
    }
    if (!needQuotes)
    {
        out << field;
        return;
    }
    out << '"';
    for (const char c : field)
    {
        if (c == '"')
        {
            out << "\"\"";
        }
        else
        {
            out << c;
        }
    }
    out << '"';
}

// CSV files open directly in Excel for manual verification of the loaded old state.
static void exportOldStateToCsv(const std::string& oldStateFile)
{
    const std::string scalarsPath = oldStateFile + "_scalars.csv";
    const std::string poolsPath = oldStateFile + "_pools.csv";
    const std::string liquiditiesPath = oldStateFile + "_liquidities.csv";

    {
        std::ofstream out(scalarsPath);
        if (!out)
        {
            throw std::runtime_error("Failed to create " + scalarsPath);
        }
        out << "field,value\n";
        out << "swapFeeRate," << old_swapFeeRate << "\n";
        out << "investRewardsFeeRate," << old_investRewardsFeeRate << "\n";
        out << "shareholderFeeRate," << old_shareholderFeeRate << "\n";
        out << "poolCreationFeeRate," << old_poolCreationFeeRate << "\n";
        out << "investRewardsId," << test_utils::idToIdentity(old_investRewardsId) << "\n";
        out << "investRewardsEarnedFee," << old_investRewardsEarnedFee << "\n";
        out << "investRewardsDistributedAmount," << old_investRewardsDistributedAmount << "\n";
        out << "shareholderEarnedFee," << old_shareholderEarnedFee << "\n";
        out << "shareholderDistributedAmount," << old_shareholderDistributedAmount << "\n";
        out << "mPoolBasicStates.capacity," << QSWAP_OLD_MAX_POOL << "\n";
        out << "mLiquidities.capacity," << old_mLiquidities.capacity() << "\n";
        out << "mLiquidities.population," << old_mLiquidities.population() << "\n";
        out << "qxFeeRate," << old_qxFeeRate << "\n";
        out << "burnFeeRate," << old_burnFeeRate << "\n";
        out << "qxEarnedFee," << old_qxEarnedFee << "\n";
        out << "qxDistributedAmount," << old_qxDistributedAmount << "\n";
        out << "burnEarnedFee," << old_burnEarnedFee << "\n";
        out << "burnedAmount," << old_burnedAmount << "\n";
        out << "cachedIssuanceFee," << old_cachedIssuanceFee << "\n";
        out << "cachedTransferFee," << old_cachedTransferFee << "\n";
    }

    {
        std::ofstream out(poolsPath);
        if (!out)
        {
            throw std::runtime_error("Failed to create " + poolsPath);
        }
        out << "pool_slot,pool_id,reserved_qu_amount,reserved_asset_amount,total_liquidity,active\n";
        for (uint64 slot = 0; slot < QSWAP_OLD_MAX_POOL; ++slot)
        {
            const OldPoolBasicState& pool = old_mPoolBasicStates.get(slot);
            const bool active = !isZeroId(pool.poolID);
            out << slot << ',';
            writeCsvEscaped(out, test_utils::idToIdentity(pool.poolID));
            out << ',' << pool.reservedQuAmount << ',' << pool.reservedAssetAmount << ','
                << pool.totalLiquidity << ',' << (active ? 1 : 0) << '\n';
        }
    }

    uint64 liquidityRows = 0;
    {
        std::ofstream out(liquiditiesPath);
        if (!out)
        {
            throw std::runtime_error("Failed to create " + liquiditiesPath);
        }
        out << "pool_slot,pool_id,entity_id,liquidity\n";
        for (uint64 slot = 0; slot < QSWAP_OLD_MAX_POOL; ++slot)
        {
            const OldPoolBasicState& pool = old_mPoolBasicStates.get(slot);
            if (isZeroId(pool.poolID))
            {
                continue;
            }

            const id& poolID = pool.poolID;
            sint64 elementIndex = old_mLiquidities.headIndex(poolID, 0);
            while (elementIndex != NULL_INDEX)
            {
                const OldLiquidityInfo& entry = old_mLiquidities.element(elementIndex);
                if (entry.liquidity != 0)
                {
                    out << slot << ',';
                    writeCsvEscaped(out, test_utils::idToIdentity(poolID));
                    out << ',';
                    writeCsvEscaped(out, test_utils::idToIdentity(entry.entity));
                    out << ',' << entry.liquidity << '\n';
                    ++liquidityRows;
                }
                elementIndex = old_mLiquidities.nextElementIndex(elementIndex);
            }
        }
    }

    std::cout << "Exported old state for Excel review:" << std::endl;
    std::cout << "  " << scalarsPath << std::endl;
    std::cout << "  " << poolsPath << " (" << QSWAP_OLD_MAX_POOL << " pool slots)" << std::endl;
    std::cout << "  " << liquiditiesPath << " (" << liquidityRows << " liquidity rows)" << std::endl;
}

// New QSWAP: one Collection PoV per (pool, LP). Old QSWAP used poolID alone as PoV.
inline id liquidityPov(const id& poolID, const id& entity, id& r)
{
    r = entity;
    r.u64._0 ^= poolID.u64._0;
    r.u64._1 ^= poolID.u64._1;
    r.u64._2 ^= poolID.u64._2;
    r.u64._3 ^= poolID.u64._3;
    return r;
}

// Rebuild mLiquidities: old PoV = poolID with {entity, liquidity}; new PoV = poolID ^ entity, LiquidityInfo per LP.
static void migrateLiquidities(
    Collection<LiquidityInfo, QSWAP_NEW_MAX_POOL * QSWAP_MAX_USER_PER_POOL>& new_mLiquidities)
{
    new_mLiquidities.reset();

    for (uint64 poolSlot = 0; poolSlot < QSWAP_NEW_MAX_POOL; ++poolSlot)
    {
        const OldPoolBasicState& pool = old_mPoolBasicStates.get(poolSlot);
        if (isZeroId(pool.poolID))
        {
            continue;
        }

        const id& poolID = pool.poolID;
        sint64 elementIndex = old_mLiquidities.headIndex(poolID, 0);
        while (elementIndex != NULL_INDEX)
        {
            const OldLiquidityInfo& oldEntry = old_mLiquidities.element(elementIndex);

            if (oldEntry.liquidity != 0)
            {
                id povScratch;
                const id newPov = liquidityPov(poolID, oldEntry.entity, povScratch);

                LiquidityInfo newEntry;
                newEntry.liquidity = oldEntry.liquidity;
                newEntry.feeDebtX64 = uint128();
                newEntry.accumulatedFee = 0;

                // Priority is unused when each PoV has a single element (same as live QSWAP: add(..., 0)).
                if (new_mLiquidities.add(newPov, newEntry, 0) == NULL_INDEX)
                {
                    throw std::runtime_error(
                        "Failed to migrate liquidity: new collection is full "
                        "(pool slot " + std::to_string(poolSlot) + ").");
                }
            }

            elementIndex = old_mLiquidities.nextElementIndex(elementIndex);
        }
    }
}

// Write migrated state to a file
void writeNewState(const std::string& filename)
{
    for (uint64 i = 0; i < QSWAP_NEW_MAX_POOL; ++i)
    {
        const OldPoolBasicState& oldPool = old_mPoolBasicStates.get(i);
        PoolBasicState newPool;
        newPool.poolID = oldPool.poolID;
        newPool.reservedQuAmount = oldPool.reservedQuAmount;
        newPool.reservedAssetAmount = oldPool.reservedAssetAmount;
        newPool.totalLiquidity = oldPool.totalLiquidity;
        newPool.accFeePerLPX64 = uint128();
        new_mPoolBasicStates.set(i, newPool);
    }

    std::cout << "Migrating liquidities..." << std::endl;
    std::cout.flush();
    migrateLiquidities(new_mLiquidities);
    std::cout << "Writing " << filename << "..." << std::endl;
    std::cout.flush();

    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile)
    {
        throw std::runtime_error("Failed to open the new state file: " + filename);
    }

    WRITE_STATE(outfile, old_swapFeeRate);
    WRITE_STATE(outfile, old_investRewardsFeeRate);
    WRITE_STATE(outfile, old_shareholderFeeRate);
    WRITE_STATE(outfile, old_poolCreationFeeRate);

    WRITE_STATE(outfile, old_investRewardsId);
    WRITE_STATE(outfile, old_investRewardsEarnedFee);
    WRITE_STATE(outfile, old_investRewardsDistributedAmount);

    WRITE_STATE(outfile, old_shareholderEarnedFee);
    WRITE_STATE(outfile, old_shareholderDistributedAmount);

    WRITE_STATE(outfile, new_mPoolBasicStates);
    WRITE_STATE(outfile, new_mLiquidities);

    WRITE_STATE(outfile, old_qxFeeRate);
    WRITE_STATE(outfile, old_burnFeeRate);

    WRITE_STATE(outfile, old_qxEarnedFee);
    WRITE_STATE(outfile, old_qxDistributedAmount);

    WRITE_STATE(outfile, old_burnEarnedFee);
    WRITE_STATE(outfile, old_burnedAmount);

    WRITE_STATE(outfile, old_cachedIssuanceFee);
    WRITE_STATE(outfile, old_cachedTransferFee);

    outfile.close();
}

int main()
{
    try
    {
        const std::string oldStateFile = "contract0013.214";
        const std::string newStateFile = "contract0013.215.new";

        std::cout << "Reading " << oldStateFile << "..." << std::endl;
        std::cout.flush();
        readOldState(oldStateFile);

        exportOldStateToCsv(oldStateFile);

        writeNewState(newStateFile);

        std::cout << "Migration completed successfully." << std::endl;
        std::cout.flush();
        std::cout << "  Old state: " << oldStateFile << std::endl;
        std::cout << "  New state: " << newStateFile << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
