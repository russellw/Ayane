// Mostly a drop-in replacement for std::vector, using the local allocator that is faster than a general-purpose heap allocator.
// Unlike std::vector, it doesn't have a separate at() function with bounds checking. Instead, the subscript operator checks index
// bounds, only in debug build, using assert().

// Usual caveat: Don't do anything that might cause reallocation (such as adding elements without having previously reserved space)
// while holding a live iterator to the vector, or a live reference to an element.

// Unusual caveats:

// Because it requires the local allocator to have already been initialized, and C++ does not specify the order in which global
// initializers in different modules are run, a vec may not be a global or static variable.

// While elements may contain pointers to other chunks of memory they own, they must not contain internal pointers to other parts of
// the same object. This means elements can be moved around with memmove, and in particular with realloc, which improves performance
// in some cases.

// Anything which doesn't meet these requirements, should use std::vector instead.
template <class T> struct vec {
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	using value_type = T;
	using reference = T&;
	using const_reference = const T&;

	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
	// The current state of the vector consists of a pointer to a block of allocated memory, the capacity (how much space, as a
	// multiple of the element size, has been allocated), and quantity (how much of the space consists of actual valid elements; the
	// rest is uninitialized memory that serves as a buffer to make it efficient to add new elements one at a time).
	uint32_t cap;
	uint32_t qty;
	uint32_t o;

	// Initialize the vector, only for use in constructors; assumes in particular that the pointer to allocated memory is not yet
	// initialized.
	void init(size_t n) {
		// TODO: if n == 0, default to some more predictive capacity?
		cap = n;
		qty = n;
		o = heap->alloc(cap * sizeof(T));
	}

	// Turn some elements back into uninitialized memory.
	void del(T* i, T* j) {
		// TODO: more efficient to free in reverse order?
		while (i != j) i++->~T();
	}

public:
	// Constructors use placement new to initialize elements where necessary with copies of source elements.
	// TODO: constructor that takes estimated initial capacity
	explicit vec(size_t n = 0) {
		init(n);
		for (auto i = begin(), e = end(); i != e; ++i) new (i) T;
	}

	explicit vec(size_t n, const T& b) {
		init(n);
		for (auto i = begin(), e = end(); i != e; ++i) new (i) T(b);
	}

	explicit vec(T* first, T* last) {
		init(last - first);
		auto i = begin();
		for (auto j = first; j != last; ++j) new (i++) T(*j);
	}

	explicit vec(std::initializer_list<T> b) {
		init(b.size());
		auto i = begin();
		for (auto& x: b) new (i++) T(x);
	}

	vec(const vec& b) {
		init(b.qty);
		auto i = begin();
		for (auto& x: b) new (i++) T(x);
	}

	// Destructor calls element destructors, but only on those elements that have actually been initialized, i.e. up to quantity,
	// not capacity.
	~vec() {
		del(begin(), end());
		heap->free(o, cap * sizeof(T));
	}

	// Reserve is used internally by other functions, but following std::vector, it is also made available in the API, where it is
	// semantically mostly a no-op, but serves as an optimization hint. The case where it is important for correctness is if you
	// want to do something like adding new elements while holding a live iterator to the vector or a live reference to an element;
	// reserving enough space in advance, can ensure that reallocation doesn't need to happen on the fly.
	void reserve(size_t n) {
		if (n <= cap) return;

		// Make sure adding one element at a time is amortized constant time.
		auto cap1 = max(n, (size_t)cap * 2);

		// Realloc is okay because of the precondition that elements have no internal pointers. It is theoretically possible for
		// realloc to be inefficient here because, knowing nothing about the semantics of vectors, it must (if actual reallocation
		// is needed) memcpy up to capacity, not just quantity. But in practice, almost all reallocations will be caused by an
		// element being added at the end, so quantity will be equal to capacity anyway.
		o = heap->realloc(o, cap * sizeof(T), cap1 * sizeof(T));

		// Update capacity. Quantity is unchanged; that's for the caller to figure out.
		cap = cap1;
	}

	void push_back(const T& x) {
		reserve(qty + 1);
		new (end()) T(x);
		++qty;
	}

	void pop_back() {
		assert(qty);
		--qty;
		end()->~T();
	}

	vec& operator=(const vec& b) {
		if (this == &b) return *this;

		// Free the existing elements.
		del(begin(), end());

		// Make room for new elements.
		reserve(b.qty);

		// Assign the new elements. These can be objects with pointers to data they own (just not internal pointers), so it cannot
		// be done with memcpy. The assignment operator must leave the original object untouched, so it cannot be done with a move
		// constructor. But we have already freed the existing elements (thereby turning the entire array into uninitialized
		// memory), so it can be done with placement new calling a copy constructor.

		// An alternative approach would have used the element assignment operator up to min(qty, b.qty). However, this would have
		// made the code more complicated (two different mop-up cases to consider, depending on which vector was larger) and would
		// almost certainly have made it no faster.
		qty = b.qty;
		auto i = begin();
		for (auto& x: b) new (i++) T(x);
		return *this;
	}

	void insert(T* position, const T* first, const T* last) {
		assert(begin() <= position && position <= end());
		auto i = position - begin();

		assert(first <= last);
		auto n = last - first;

		reserve(qty + n);
		position = begin() + i;
		memmove(position + n, position, (qty - i) * sizeof(T));
		auto r = position;
		for (auto p = first; p != last; ++p) new (r++) T(*p);
		qty += n;
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
		qty -= last - first;
	}

	void erase(T* position) {
		erase(position, position + 1);
	}

	// This could be used to either expand or shrink the vector. At the moment it is only used for shrinking, so that is the only
	// supported case.
	void resize(size_t n) {
		assert(n <= qty);
		del(begin() + n, end());
		qty = n;
	}

	void clear() {
		resize(0);
	}

	// Capacity.
	size_t size() const {
		return qty;
	}

	bool empty() const {
		return !qty;
	}

	// Data access.
	T* data() {
		return begin();
	}

	const T* data() const {
		return begin();
	}

	// Iterators.
	iterator begin() {
		return (T*)heap->ptr(o);
	}

	const_iterator begin() const {
		return (T*)heap->ptr(o);
	}

	iterator end() {
		return begin() + qty;
	}

	const_iterator end() const {
		return begin() + qty;
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

	// Element access.
	T& operator[](size_t i) {
		assert(i < qty);
		return begin()[i];
	}

	const T& operator[](size_t i) const {
		assert(i < qty);
		return begin()[i];
	}

	T& front() {
		assert(qty);
		return *begin();
	}

	const T& front() const {
		assert(qty);
		return *begin();
	}

	T& back() {
		assert(qty);
		return begin()[qty - 1];
	}

	const T& back() const {
		assert(qty);
		return begin()[qty - 1];
	}
};

template <class T> size_t hash(const vec<T>& a) {
	size_t h = 0;
	for (auto& x: a) h = hashCombine(h, hash(x));
	return h;
}

template <class T> bool operator==(const vec<T>& a, const vec<T>& b) {
	auto n = a.size();
	if (n != b.size()) return 0;
	for (size_t i = 0; i != n; ++i)
		if (a[i] != b[i]) return 0;
	return 1;
}

template <class T> bool operator!=(const vec<T>& a, const vec<T>& b) {
	return !(a == b);
}

template <class T> void print(const vec<T>& a) {
	putchar('[');
	bool more = 0;
	for (auto& x: a) {
		if (more) print(", ");
		more = 1;
		print(x);
	}
	putchar(']');
}
