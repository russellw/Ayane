// TODO: document rationale
template <class K, class V, class T, class Cmp> class Set {
	size_t cap = 4;
	size_t qty;
	T** entries = (T**)calloc(cap, sizeof *entries);

	static size_t slot(T** entries, size_t cap, K tag, V v, size_t n) {
		size_t mask = cap - 1;
		auto i = Cmp::hash(tag, v, n) & mask;
		while (entries[i] && !Cmp::eq(tag, v, n, entries[i])) i = (i + 1) & mask;
		return i;
	}
	static size_t slot(T** entries, size_t cap, T* a) {
		size_t mask = cap - 1;
		auto i = Cmp::hash(a) & mask;
		while (entries[i] && !Cmp::eq(a, entries[i])) i = (i + 1) & mask;
		return i;
	}

	void expand() {
		assert(isPow2(cap));
		auto cap1 = cap * 2;
		auto entries1 = (T**)calloc(cap1, sizeof *entries);
		// TODO: check generated code
		for (auto i = entries, e = entries + cap; i < e; ++i) {
			auto a = *i;
			if (a) entries1[slot(entries1, cap1, a)] = a;
		}
		free(entries);
		cap = cap1;
		entries = entries1;
	}

public:
	T* intern(K tag, V v, size_t n) {
		auto i = slot(entries, cap, tag, v, n);

		// If we have seen this before, return the existing object
		if (entries[i]) {
			// TODO: cache result in local?
			clear(v);
			return entries[i];
		}

		// Expand the hash table if necessary
		if (++qty > cap * 3 / 4) {
			expand();
			i = slot(entries, cap, tag, v, n);
			assert(!entries[i]);
		}

		// Make a new object and add to hash table
		return entries[i] = Cmp::make(tag, v, n);
	}
};
