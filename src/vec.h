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
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	using value_type = T;
	using reference = T&;
	using const_reference = const T&;

	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	uint32_t n, cap;
	// TODO: optimize for small sizes
	T* data;

	// Initialize the vector, only for use in constructors; assumes in particular that the pointer to allocated memory is not yet
	// initialized
	void init(size_t o) {
		// TODO: if o == 0, default to some more predictive capacity?
		cap = o;
		n = o;
		data = (T*)malloc(cap * sizeof(T));
	}

	// Turn some elements back into uninitialized memory
	void del(T* i, T* j) {
		// TODO: more efficient to free in reverse order?
		while (i < j) i++->~T();
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

	Vec(const Vec& b) {
		// TODO: disable?
		init(b.n);
		auto i = begin();
		for (auto& x: b) new (i++) T(x);
	}

	// Destructor calls element destructors, but only on those elements that have actually been initialized, i.e. up to size, not
	// capacity.
	~Vec() {
		del(begin(), end());
		free(data);
	}

	// Reserve is used internally by other functions, but following std::vector, it is also made available in the API, where it is
	// semantically mostly a no-op, but serves as an optimization hint. The case where it is important for correctness is if you
	// want to do something like adding new elements while holding a live iterator to the vector or a live reference to an element;
	// reserving enough space in advance, can ensure that reallocation doesn't need to happen on the fly.
	void reserve(size_t o) {
		if (o <= cap) return;

		// Make sure adding one element at a time is amortized constant time
		auto cap1 = max(o, (size_t)cap * 2);

		// Realloc is okay because of the precondition that elements have no internal pointers. It is theoretically possible for
		// realloc to be inefficient here because, knowing nothing about the semantics of vectors, it must (if actual reallocation
		// is needed) memcpy up to capacity, not just size. But in practice, almost all reallocations will be caused by an element
		// being added at the end, so size will be equal to capacity anyway.
		data = (T*)realloc(data, cap1 * sizeof(T));

		// Update capacity. Size is unchanged; that's for the caller to figure out.
		cap = cap1;
	}

	void add(const T& x) {
		reserve(n + 1);
		new (end()) T(x);
		++n;
	}

	void pop_back() {
		assert(n);
		--n;
		end()->~T();
	}

	Vec& operator=(const Vec& b) {
		if (this == &b) return *this;

		// Free the existing elements
		del(begin(), end());

		// Make room for new elements
		reserve(b.n);

		// Assign the new elements. These can be objects with pointers to data they own (just not internal pointers), so it cannot
		// be done with memcpy. The assignment operator must leave the original object untouched, so it cannot be done with a move
		// constructor. But we have already freed the existing elements (thereby turning the entire array into uninitialized
		// memory), so it can be done with placement new calling a copy constructor.

		// An alternative approach would have used the element assignment operator up to min(n, b.n). However, this would have made
		// the code more complicated (two different mop-up cases to consider, depending on which vector was larger) and would almost
		// certainly have made it no faster.
		n = b.n;
		auto i = begin();
		for (auto& x: b) new (i++) T(x);
		return *this;
	}

	void insert(T* position, const T* first, const T* last) {
		assert(begin() <= position && position <= end());
		auto i = position - begin();

		assert(first <= last);
		auto o = last - first;

		reserve(n + o);
		position = begin() + i;
		memmove(position + o, position, (n - i) * sizeof(T));
		auto r = position;
		for (auto p = first; p != last; ++p) new (r++) T(*p);
		n += o;
	}

	void insert(T* position, const T& x) {
		insert(position, &x, &x + 1);
	}

	void erase(T* first, T* last) {
		assert(begin() <= first && first <= end());
		assert(begin() <= last && last <= end());
		assert(first <= last);
		del(first, last);
		memmove(first, last, (end() - last) * sizeof(T));
		n -= last - first;
	}

	void erase(T* position) {
		erase(position, position + 1);
	}

	// This could be used to either expand or shrink the vector. At the moment it is only used for shrinking, so that is the only
	// supported case.
	void resize(size_t o) {
		// TODO: inline
		assert(o <= n);
		del(begin() + o, end());
		n = o;
	}

	void clear() {
		resize(0);
	}

	// Capacity
	size_t size() const {
		// TODO: inline
		return n;
	}

	bool empty() const {
		// TODO: inline
		return !n;
	}

	// Iterators
	iterator begin() {
		return data;
	}

	const_iterator begin() const {
		return data;
	}

	iterator end() {
		return begin() + n;
	}

	const_iterator end() const {
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

	T& front() {
		assert(n);
		return *begin();
	}

	const T& front() const {
		assert(n);
		return *begin();
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
	bool has(const T& x) {
		for (auto& y: *this)
			if (y == x) return 1;
		return 0;
	}
};

template <class K, class T> bool get(K x, T& y, const Vec<pair<K, T>>& m) {
	for (auto xy: m)
		if (xy.first == x) {
			y = xy.second;
			return 1;
		}
	return 0;
}

template <class T> size_t hash(const Vec<T>& a) {
	size_t h = 0;
	for (auto& x: a) h = hashCombine(h, hash(x));
	return h;
}

template <class T> bool operator==(const Vec<T>& a, const Vec<T>& b) {
	auto o = a.size();
	if (o != b.size()) return 0;
	for (size_t i = 0; i < o; ++i)
		if (at(a, i) != b[i]) return 0;
	return 1;
}

template <class T> bool operator!=(const Vec<T>& a, const Vec<T>& b) {
	return !(a == b);
}

template <class T> void print(const Vec<T>& a) {
	putchar('[');
	bool more = 0;
	for (auto& x: a) {
		if (more) print(", ");
		more = 1;
		print(x);
	}
	putchar(']');
}
