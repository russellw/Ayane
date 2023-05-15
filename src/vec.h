// Partly a replacement for std::vector

// Unlike std::vector, doesn't have a separate at() function with bounds checking. Instead, the subscript operator checks index
// bounds, only in debug build, using assert().

// Usual caveat: Don't do anything that might cause reallocation (such as adding elements without having previously reserved space)
// while holding a live iterator to the vector, or a live pointer to an element

// Unusual caveat: While elements may contain pointers to other chunks of memory they own, they must not contain internal pointers
// to other parts of the same object. This means elements can be moved around with memmove, and in particular with realloc, which
// improves performance in some cases.

// Anything which doesn't meet this requirement, should use std::vector instead
template <class T> struct Vec {
	// TODO: simplify
	using reverse_iterator = std::reverse_iterator<T*>;
	using const_reverse_iterator = std::reverse_iterator<const T*>;

	uint32_t n, cap;
	// TODO: optimize for small sizes
	T* data;

	Vec(const Vec&) = delete;
	Vec& operator=(const Vec&) = delete;

	// Initialize the vector, only for use in constructors; assumes in particular that the pointer to allocated memory is not yet
	// initialized
	void init(size_t o) {
		// TODO: if o == 0, default to some more predictive capacity?
		cap = o;
		n = o;
		data = (T*)malloc(cap * sizeof(T));
	}

	// Constructors use placement new to initialize elements where necessary with copies of source elements
	// TODO: constructor that takes estimated initial capacity
	explicit Vec(size_t o = 0) {
		init(o);
		for (auto i = begin(), e = end(); i < e; ++i) new (i) T;
	}

	explicit Vec(size_t o, const T& b) {
		init(o);
		for (auto i = begin(), e = end(); i < e; ++i) new (i) T(b);
	}

	explicit Vec(T* first, T* last) {
		init(last - first);
		auto i = begin();
		for (auto j = first; j != last; ++j) new (i++) T(*j);
	}

	explicit Vec(std::initializer_list<T> b) {
		init(b.size());
		auto i = begin();
		for (auto& x: b) new (i++) T(x);
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

	const T* begin() const {
		return data;
	}

	T* end() {
		return begin() + n;
	}

	const T* end() const {
		return begin() + n;
	}

	reverse_iterator rbegin() {
		return reverse_iterator(end());
	}

	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(end());
	}

	reverse_iterator rend() {
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const {
		return const_reverse_iterator(begin());
	}

	// Element access
	T& operator[](size_t i) {
		assert(i < n);
		return begin()[i];
	}

	const T& operator[](size_t i) const {
		assert(i < n);
		return begin()[i];
	}

	T& back() {
		assert(n);
		return begin()[n - 1];
	}

	const T& back() const {
		assert(n);
		return begin()[n - 1];
	}

	// Etc
	// TODO: use this
	bool has(const T& x) {
		for (auto& y: *this)
			if (y == x) return 1;
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
