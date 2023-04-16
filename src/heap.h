// This memory manager is used instead of standard library malloc, with the following API differences:

// Alignment is specified as a template parameter. 8-byte alignment is confined to heaps that specifically need it; the default is
// 4-byte alignment.

// Instead of pointers, it returns 32-bit offsets, which are decoded into pointers only as needed. (Sizes of allocated memory chunks
// are still measured in bytes as usual.) The offsets are scaled to the heap alignment, so a 4-byte heap can be up to 16 gigabytes.

// Free requires the size of the allocated block as a parameter.

// Heaps are not shared between threads. Each thread must have its own.
void* reserve(size_t n);
void commit(void* p, size_t n);

template <size_t alignment = 4> class Heap {
	static const size_t heapSize = 1ULL << 33;

	// The entire heap is divided into slabs of a fixed size. Everything is measured from the beginning of the heap, including the
	// heap data structure itself, so the first slab (or several slabs, depending on how big they are) is occupied by the heap data
	// structure.
	static const int slabBits = 18;
	static const size_t slabSize = 1 << slabBits;
	static const size_t nslabs = heapSize / slabSize;

	// Each slab can contain a free list of memory blocks of a particular size, or can be part of a single large (multi-slab)
	// allocation. That is, for allocations smaller than a slab, free lists are segregated by size and slab.
	struct Slab {
		uint32_t allocated;
		uint32_t freeList;
		uint32_t sc;
	};

	// It would be inefficient to keep a separate free list for every possible allocation size. Instead, allocations are classified
	// into groups of size classes:

	// Small. From one to several words (need to round up to the nearest word anyway for alignment), each size gets its own free
	// list; this is worth doing because most allocations are small.
	static const int smallBits = 4;

	// Medium. An allocation larger than small, but no larger than half a slab, is rounded up to a power of two. The rounding up
	// wastes some memory, but fully uses each slab, and many medium-sized allocations e.g. for hash tables are naturally powers of
	// two anyway.
	static const int mediumBits = slabBits - 1 - smallBits;

	// Large. Allocations larger than half a slab, are rounded up to a whole number of slabs, so do not have a free list.

	// Track how many slabs we have allocated from the operating system so far.
	uint32_t top = divUp(sizeof(Heap), slabSize);

	// Each size class keeps the index (not offset) of the slab it is currently allocating from. The heap data structure occupies at
	// least one slab, therefore zero is not a valid slab index, so serves as a marker for size classes that do not currently have a
	// slab.
	uint32_t sizeClasses[1 + (1 << smallBits) + mediumBits];

	// Metadata for each slab.
	Slab slabs[nslabs];

	// Figure out the size class for an allocation, and round up the size accordingly. A negative return value means this is a large
	// allocation.
	static size_t classify(size_t& n) {
		static_assert(alignment >= 4);
		static_assert(isPow2(alignment));
		assert(n);

		// Measure size in words for this.
		auto w = divUp(n, alignment);

		// Small sizes get their own classes.
		if (w <= 1 << smallBits) {
			n = w * alignment;
			return w;
		}

		// Medium sizes are rounded up to the next power of two.
		for (int i = smallBits + 1; 1 << i < slabSize / alignment; ++i)
			if (w <= 1 << i) {
				n = (1 << i) * alignment;
				return (1 << smallBits) - smallBits + i;
			}

		// Large allocations are rounded up to a whole number of slabs.
		n = roundUp(n, slabSize);
		return 0;
	}

	bool areFreeSlabs(size_t i, size_t n) {
		for (auto j = i; j != i + n; ++j)
			if (slabs[j].allocated | slabs[j].sc) return 0;
		return 1;
	}

	size_t allocSlabs(size_t n) {
		assert(top <= nslabs);
		assert(n);
		for (auto i = divUp(sizeof(Heap), slabSize); i + n <= top; ++i)
			if (areFreeSlabs(i, n)) return i;
		if (top + n > nslabs) err("Out of memory");
		size_t i = top;
		commit((char*)this + i * slabSize, n * slabSize);
		top += n;
		return i;
	}

public:
	static Heap* make() {
		auto p = (Heap*)reserve(heapSize);
		commit(p, sizeof *p);
		new (p) Heap;
		return p;
	}

	// Turn an offset into a pointer for actual use.
	void* ptr(size_t o) const {
		assert(o <= top * slabSize / alignment);
		return (char*)this + o * alignment;
	}

	// Turn a pointer back into an offset, suitable for compact storage, also for passing to realloc or free.
	// TODO: still  needed?
	size_t offset(const void* p) const {
		auto q = (char*)p;
		assert((char*)this <= q);

		// Assign the difference to an unsigned variable and then perform the division, because ptrdiff_t is a signed type, but
		// unsigned division is slightly faster.
		size_t i = q - (char*)this;
		return i / alignment;
	}

	size_t alloc(size_t n) {
		if (!n) return 0;

		// Figure out the size class, and round the size up appropriately.
		auto sc = classify(n);
		assert(n % alignment == 0);
		assert(sc < sizeof sizeClasses / sizeof *sizeClasses);

		// Index of the (first) slab.
		size_t i;

		// Offset of the allocated memory.
		size_t o;

		// Is it a defined size class?
		if (sc) {
			i = sizeClasses[sc];

			// If we don't have a current slab for that size class.
			if (!i) {
				// Try to find one.
				for (i = divUp(sizeof(Heap), slabSize); i != top; ++i)
					if (slabs[i].sc == sc && slabs[i].freeList) break;
				if (i == top) {
					// Otherwise allocate one.
					i = allocSlabs(1);
					slabs[i].allocated = 0;
					slabs[i].sc = sc;

					// Initialize the free list.
					o = i * slabSize / alignment;
					slabs[i].freeList = o;
					for (auto e = o + slabSize / alignment; o + n / alignment <= e; o += n / alignment)
						*((uint32_t*)ptr(o)) = o + n / alignment;

					// And null terminate it.
					*((uint32_t*)ptr(o - n / alignment)) = 0;
				}
				sizeClasses[sc] = i;
			}
			++slabs[i].allocated;

			// Get the next free memory block.
			o = slabs[i].freeList;

			// Update the free list to point to the one after that.
			slabs[i].freeList = *((uint32_t*)ptr(o));

			// If there isn't one, this block is full.
			if (!slabs[i].freeList) sizeClasses[sc] = 0;
		} else {
			// Allocate a block of slabs.
			i = allocSlabs(n / slabSize);
			for (auto j = i, e = i + n / slabSize; j != e; ++j) {
				assert(!slabs[j].allocated);
				assert(!slabs[j].sc);
				slabs[j].allocated = 1;
			}
			o = i * slabSize / alignment;
		}
#ifdef DEBUG
		memset(ptr(o), 0xcc, n);
#endif
		return o;
	}

	size_t calloc(size_t n) {
		auto r = alloc(n);
		memset(ptr(r), 0, n);
		return r;
	}

	size_t realloc(size_t o, size_t old, size_t n) {
		auto r = alloc(n);
		memcpy(ptr(r), ptr(o), old);
		free(o, old);
		return r;
	}

	void free(size_t o, size_t n) {
		if (!o) {
			assert(!n);
			return;
		}

		// Figure out the size class, and round the size up appropriately.
		auto sc = classify(n);
		assert(n % alignment == 0);
		assert(sc < sizeof sizeClasses / sizeof *sizeClasses);
#ifdef DEBUG
		memset(ptr(o), 0xdd, n);
#endif

		// Index of the (first) slab.
		auto i = o / (slabSize / alignment);

		// Is it a defined size class?
		if (sc) {
			assert(slabs[i].sc == sc);

			// Add the memory block to the free list.
			*((uint32_t*)ptr(o)) = slabs[i].freeList;
			slabs[i].freeList = o;

			// Mark the slab less allocated.
			assert(slabs[i].allocated);
			--slabs[i].allocated;

			// If this slab is now empty, and is not depended on as the current slab for its size class, free it up completely.
			if (!slabs[i].allocated && sizeClasses[sc] != i) slabs[i].sc = 0;
		} else {
			// Free a block of slabs.
			for (auto j = i, e = i + n / slabSize; j != e; ++j) {
				assert(slabs[j].allocated == 1);
				assert(!slabs[j].sc);
				slabs[j].allocated = 0;
			}
		}
	}

	size_t size() {
		return top * alignment;
	}

	void check() {
#ifdef DEBUG
#endif
	}

	void dump() {
#ifdef DEBUG
		putchar('\n');
#endif
	}
};

// Not the only heap, but the one used by default.
extern Heap<>* heap;
