template <class K, class V> struct map {
	using T = pair<K, V>;
	// TODO: still needed?
	using entry = pair<bool, T>;

private:
	uint32_t cap;
	uint32_t qty;
	entry* entries;

	static size_t slot(entry* entries, size_t cap, const K& k) {
		assert(cap);
		size_t mask = cap - 1;
		auto i = hash(k) & mask;
		while (entries[i].first && entries[i].second.first != k) i = (i + 1) & mask;
		return i;
	}

	static void copy(entry* entries, size_t cap, entry* entries1, size_t cap1) {
		for (auto p = entries, e = p + cap; p != e; ++p) {
			if (!p->first) continue;
			auto i = slot(entries1, cap1, p->second.first);
			assert(!entries1[i].first);
			entries1[i].first = 1;
			new (&entries1[i].second) T(p->second);
		}
	}

	void expand() {
		size_t cap1 = cap ? cap * 2 : 4;
		auto entries1 = (entry*)calloc(cap1, sizeof(entry));
		copy(entries, cap, entries1, cap1);
		free(entries);
		cap = cap1;
		entries = entries1;
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

	map() {
		cap = 0;
		qty = 0;
		entries = 0;
	}

	map(const map& b) {
		cap = b.cap;
		qty = b.qty;
		entries = (entry*)calloc(cap, sizeof(entry));
		// TODO: straight memcpy could be used here
		copy(b.entries, b.cap, entries, cap);
	}

	// TODO: extend this to other  containers
	map(map&& b) {
		cap = b.cap;
		qty = b.qty;
		entries = b.entries;
		b.cap = 0;
		b.entries = 0;
	}

	map& operator=(const map& b) {
		if (this == &b) return *this;
		clear();
		qty = b.qty;
		if (cap < b.cap) {
			free(entries);
			cap = b.cap;
			entries = (entry*)calloc(cap, sizeof(entry));
		}
		copy(b.entries, b.cap, entries, cap);
		return *this;
	}

	~map() {
		for (auto p = entries, e = p + cap; p != e; ++p)
			if (p->first) p->second.~T();
		free(entries);
	}

	// Same as the standard library API: Check whether a key exists in the map
	bool count(const K& k) const {
		// TODO: init to cap=4 to eliminate special case?
		if (!cap) return 0;

		auto i = slot(entries, cap, k);
		return entries[i].first;
	}

	// Similar to the standard library API: Get the value, if you know a key is present. The validity of this is checked only in
	// debug build.
	const V& at(const K& k) const {
		auto i = slot(entries, cap, k);
		assert(entries[i].first);
		return entries[i].second.second;
	}

	// This is a new function, inspired by other languages; it checks for the existence of a key, gets the value if it exists, but
	// doesn't touch the map if not
	bool get(const K& k, V& v) const {
		// TODO: is K ever more than a scalar?
		if (!cap) return 0;

		auto i = slot(entries, cap, k);
		if (!entries[i].first) return 0;

		v = entries[i].second.second;
		return 1;
	}

	// Also new: Add a key and value, only if not already present
	bool add(const K& k, const V& v) {
		// TODO: rename v to r
		size_t i;
		if (cap) {
			i = slot(entries, cap, k);
			if (entries[i].first) return 0;
		}
		if (++qty > (size_t)cap * 3 / 4) {
			expand();
			i = slot(entries, cap, k);
			assert(!entries[i].first);
		}
		entries[i].first = 1;
		new (&entries[i].second) T(make_pair(k, v));
		return 1;
	}

	// This does the same thing as std::unordered_map operator[]. It should be used only in specialized circumstances; using it
	// elsewhere will silently yield suboptimal code, so it seems better to give it a name that can be searched for, and makes it
	// clear that it should not be used by default.
	V& gadd(const K& k) {
		size_t i;
		if (cap) {
			i = slot(entries, cap, k);
			if (entries[i].first) return entries[i].second.second;
		}
		if (++qty > (size_t)cap * 3 / 4) {
			expand();
			i = slot(entries, cap, k);
			assert(!entries[i].first);
		}
		entries[i].first = 1;
		new (&entries[i].second.first) K(k);
		new (&entries[i].second.second) V;
		return entries[i].second.second;
	}

	void clear() {
		for (auto p = entries, e = p + cap; p != e; ++p)
			if (p->first) {
				p->first = 0;
				p->second.~T();
			}
		qty = 0;
	}

	// Capacity
	size_t size() const {
		return qty;
	}

	bool empty() const {
		return !qty;
	}

	// Iterators
	iterator begin() const {
		auto p = entries;
		auto e = p + cap;
		while (p != e && !p->first) ++p;
		return iterator(p, e);
	}

	iterator end() const {
		auto e = entries + cap;
		return iterator(e, e);
	}
};

template <class K, class V> size_t hash(const map<K, V>& a) {
	auto h = a.size();
	for (auto& kv: a) h ^= hash(kv);
	return h;
}

template <class K, class V> bool operator==(const map<K, V>& a, const map<K, V>& b) {
	if (a.size() != b.size()) return 0;
	for (auto& kv: a)
		if (!b.count(kv.first)) return 0;
	return 1;
}

template <class K, class V> bool operator!=(const map<K, V>& a, const map<K, V>& b) {
	return !(a == b);
}

template <class K, class V> void print(const map<K, V>& a) {
	putchar('{');
	bool more = 0;
	for (auto& kv: a) {
		if (more) print(", ");
		more = 1;
		print(kv.first);
		print(":");
		print(kv.second);
	}
	putchar('}');
}
