// Partly a replacement for std::vector

// Unlike std::vector, doesn't have a separate at() function with bounds checking. Instead, the subscript operator checks index
// bounds, only in debug build, using assert().

// Usual caveat: Don't do anything that might cause reallocation (such as adding elements without having previously reserved space)
// while holding a live iterator to the vector, or a live pointer to an element

// Unusual caveat: While elements may contain pointers to other chunks of memory they own, they must not contain internal pointers
// to other parts of the same object. This means elements can be moved around with memmove, and in particular with realloc, which
// improves performance in some cases.

// Anything which doesn't meet this requirement, should use std::vector instead
template <class T> class Vec {
	uint32_t cap;

public:
	// TODO: simplify
	using reverse_iterator = std::reverse_iterator<T*>;

	uint32_t n;
	// TODO: optimize for small sizes
	T* data;

	Vec(const Vec&) = delete;
	Vec& operator=(const Vec&) = delete;

	// Initialize the vector, only for use in constructors; assumes in particular that the pointer to allocated memory is not yet
	// initialized
	void init(size_t o) {
		cap = o;
		n = o;
		data = (T*)malloc(cap * sizeof(T));
	}

	explicit Vec(size_t o = 0) {
		init(o);
	}

	explicit Vec(size_t o, T a) {
		init(o);
		*data = a;
	}

	~Vec() {
		free(data);
	}

	// Reserve is used internally by other functions, but following std::vector, it is also made available in the API, where it is
	// semantically mostly a no-op, but serves as an optimization hint. The case where it is important for correctness is if you
	// want to do something like adding new elements while holding a live iterator to the vector or a live reference to an element;
	// reserving enough space in advance, can ensure that reallocation doesn't need to happen on the fly.
	void reserve(size_t cap1) {
		if (cap1 <= cap) return;

		// Make sure adding one element at a time is amortized constant time
		cap1 = max(cap1, (size_t)cap * 2);

		// Realloc is okay because of the precondition that elements have no internal pointers. It is theoretically possible for
		// realloc to be inefficient here because, knowing nothing about the semantics of vectors, it must (if actual reallocation
		// is needed) memcpy up to capacity, not just size. But in practice, almost all reallocations will be caused by an element
		// being added at the end, so size will be equal to capacity anyway.
		data = (T*)realloc(data, cap1 * sizeof(T));

		// Update capacity. Size is unchanged; that's for the caller to figure out.
		cap = cap1;
	}

	void add(T x) {
		reserve(n + 1);
		new (end()) T(x);
		++n;
	}

	// Iterators
	T* begin() {
		return data;
	}

	T* end() {
		return begin() + n;
	}

	reverse_iterator rbegin() {
		return reverse_iterator(end());
	}

	reverse_iterator rend() {
		return reverse_iterator(begin());
	}

	// Element access
	T& operator[](size_t i) {
		assert(i < n);
		return begin()[i];
	}

	// Etc
	bool has(const T& x) {
		for (auto& y: *this)
			if (x == y) return 1;
		return 0;
	}
};

template <class K, class T> bool get(K x, T& y, Vec<pair<K, T>>& m) {
	for (auto xy: m)
		if (xy.first == x) {
			y = xy.second;
			return 1;
		}
	return 0;
}

template <class T> void print(Vec<T>& a) {
	putchar('[');
	bool more = 0;
	for (auto& x: a) {
		if (more) print(", ");
		more = 1;
		print(x);
	}
	putchar(']');
}
