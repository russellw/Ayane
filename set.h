template <class T> struct set {
	using entry = pair<bool, T>;

private:
	uint32_t cap;
	uint32_t qty;
	uint32_t o;

	static size_t slot(entry* entries, size_t cap, const T& x) {
		size_t mask = cap - 1;
		auto i = hash(x) & mask;
		while (entries[i].first && entries[i].second != x) i = (i + 1) & mask;
		return i;
	}

	void copy(entry* entries, size_t cap, entry* entries1, size_t cap1) {
		for (auto p = entries, e = p + cap; p != e; ++p) {
			if (!p->first) continue;
			auto i = slot(entries1, cap1, p->second);
			assert(!entries1[i].first);
			entries1[i].first = 1;
			new (&entries1[i].second) T(p->second);
		}
	}

	void expand() {
		assert(isPow2(cap));
		auto cap1 = cap * 2;
		auto o1 = heap->calloc(cap1 * sizeof(entry));
		auto entries = (entry*)heap->ptr(o);
		auto entries1 = (entry*)heap->ptr(o1);
		copy(entries, cap, entries1, cap1);
		heap->free(o, cap * sizeof(entry));
		cap = cap1;
		o = o1;
	}

public:
	struct iterator {
		using iterator_category = std::forward_iterator_tag;

		using difference_type = ptrdiff_t;

		using value_type = entry;
		using pointer = entry*;
		using reference = entry&;

	private:
		entry* p;
		entry* e;

	public:
		iterator(entry* p, entry* e): p(p), e(e) {
		}

		T& operator*() const {
			return p->second;
		}

		T* operator->() {
			return &p->second;
		}

		iterator& operator++() {
			do {
				++p;
				assert(p <= e);
			} while (p != e && !p->first);
			return *this;
		}

		iterator operator++(int) {
			auto r = *this;
			++*this;
			return r;
		}

		friend bool operator==(iterator i, iterator j) {
			return i.p == j.p;
		}

		friend bool operator!=(iterator i, iterator j) {
			return !(i == j);
		}
	};

	explicit set() {
		cap = 4;
		qty = 0;
		o = heap->calloc(cap * sizeof(entry));
	}

	set(const set& b) {
		cap = b.cap;
		qty = b.qty;
		o = heap->calloc(cap * sizeof(entry));
		auto bentries = (entry*)heap->ptr(b.o);
		auto entries = (entry*)heap->ptr(o);
		copy(bentries, b.cap, entries, cap);
	}

	set& operator=(const set& b) {
		if (this == &b) return *this;

		// In principle, this sequence of operations is not quite optimal; clear() unconditionally clears the live-entry flags
		// before it is known whether the existing table will be kept, thereby potentially generating a small amount of unnecessary
		// memory traffic. In practice, it is expected that either the set being assigned to, will be initially empty, or it will be
		// at least as big as the set being assigned from, so there is no actual inefficiency.
		clear();
		qty = b.qty;
		if (cap < b.cap) {
			heap->free(o, cap * sizeof(entry));
			cap = b.cap;
			o = heap->calloc(cap * sizeof(entry));
		}
		auto bentries = (entry*)heap->ptr(b.o);
		auto entries = (entry*)heap->ptr(o);
		copy(bentries, b.cap, entries, cap);
		return *this;
	}

	~set() {
		for (auto p = (entry*)heap->ptr(o), e = p + cap; p != e; ++p)
			if (p->first) p->second.~T();
		heap->free(o, cap * sizeof(entry));
	}

	bool count(const T& x) const {
		auto entries = (entry*)heap->ptr(o);
		auto i = slot(entries, cap, x);
		return entries[i].first;
	}

	bool add(const T& x) {
		auto entries = (entry*)heap->ptr(o);
		auto i = slot(entries, cap, x);
		if (entries[i].first) return 0;

		if (++qty > (size_t)cap * 3 / 4) {
			expand();
			entries = (entry*)heap->ptr(o);
			i = slot(entries, cap, x);
			assert(!entries[i].first);
		}
		entries[i].first = 1;
		new (&entries[i].second) T(x);
		return 1;
	}

	// When initializing a set from another set, memory is allocated up front. Here, reallocation could happen repeatedly as
	// elements are added one by one, which is inefficient. This is okay if adding multiple elements is rare, or the number of
	// elements to be added is usually small. (Including the case where many elements are presented to be added, but most turn out
	// to be already present, in which case the current code avoids unnecessarily allocating extra memory.) If adding many elements
	// in one operation turns out to be common, the code should be changed to reallocate just once.
	void add(const T* first, const T* last) {
		for (auto i = first; i != last; ++i) add(*i);
	}

	// This is inefficient; erasing an element takes O(N) time and extra memory allocation. The tradeoff is the state of the hash
	// table stays simple, so all other operations stay fast. This is a good trade if erasing elements is rare. If that turns out
	// not to be the case, a more complex design that marks the locations of erased elements will be needed.
	void erase(const T& x) {
		auto entries = (entry*)heap->ptr(o);
		auto i = slot(entries, cap, x);
		if (!entries[i].first) return;

		auto o1 = heap->calloc(cap * sizeof(entry));
		auto entries1 = (entry*)heap->ptr(o1);
		for (auto p = entries, e = p + cap; p != e; ++p) {
			if (!p->first) continue;
			if (p->second == x) continue;
			auto i = slot(entries1, cap, p->second);
			assert(!entries1[i].first);
			entries1[i].first = 1;
			new (&entries1[i].second) T(p->second);
		}
		heap->free(o, cap * sizeof(entry));
		--qty;
		o = o1;
	}

	void clear() {
		for (auto p = (entry*)heap->ptr(o), e = p + cap; p != e; ++p)
			if (p->first) {
				p->first = 0;
				p->second.~T();
			}
		qty = 0;
	}

	// Capacity.
	size_t size() const {
		return qty;
	}

	bool empty() const {
		return !qty;
	}

	// Iterators.
	iterator begin() const {
		auto p = (entry*)heap->ptr(o);
		auto e = p + cap;
		while (p != e && !p->first) ++p;
		return iterator(p, e);
	}

	iterator end() const {
		auto e = (entry*)heap->ptr(o) + cap;
		return iterator(e, e);
	}
};

template <class T> size_t hash(const set<T>& a) {
	auto h = a.size();
	for (auto& x: a) {
		// The usual hashCombine function for pairs and vectors is noncommutative in order to distinguish between permutations. But
		// that's not what we want here; two sets of the same elements in different order are equal, so we need to combine the
		// element hashes with a commutative operation.
		h ^= hash(x);
	}
	return h;
}

template <class T> bool operator==(const set<T>& a, const set<T>& b) {
	if (a.size() != b.size()) return 0;
	for (auto& x: a)
		if (!b.count(x)) return 0;
	return 1;
}

template <class T> bool operator!=(const set<T>& a, const set<T>& b) {
	return !(a == b);
}

template <class T> void print(const set<T>& a) {
	putchar('{');
	bool more = 0;
	for (auto& x: a) {
		if (more) print(", ");
		more = 1;
		print(x);
	}
	putchar('}');
}
