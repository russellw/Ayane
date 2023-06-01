template <class T, int small = 4> class Vec {
	T fixed[small];
	uint32_t cap = small;

	void init(size_t o) {
		n = o;
		if (n > small) {
			cap = n;
			data = (T*)malloc(cap * sizeof(T));
		}
	}

	void reserve(size_t cap1) {
		if (cap1 <= cap) return;
		cap = max(cap1, (size_t)cap * 2);
		if (data == fixed) {
			data = (T*)malloc(cap * sizeof(T));
			memcpy(data, fixed, n * sizeof(T));
		} else
			data = (T*)realloc(data, cap * sizeof(T));
	}

public:
	using reverse_iterator = std::reverse_iterator<T*>;

	uint32_t n;
	T* data = fixed;

	Vec(const Vec&) = delete;
	Vec& operator=(const Vec&) = delete;

	explicit Vec(size_t o = 0) { init(o); }

	explicit Vec(size_t o, T a) {
		init(o);
		*data = a;
	}

	~Vec() {
		if (data != fixed) free(data);
	}

	// Iterators
	T* begin() { return data; }
	T* end() { return begin() + n; }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	reverse_iterator rend() { return reverse_iterator(begin()); }

	// SORT
	T& operator[](size_t i) {
		assert(i < n);
		return begin()[i];
	}

	bool has(T a) {
		for (auto b: *this)
			if (a == b) return 1;
		return 0;
	}

	void add(T a) {
		reserve(n + 1);
		data[n++] = a;
	}

	void add(T* v, size_t o) {
		reserve(n + o);
		memcpy(data + n, v, o * sizeof(T));
		n += o;
	}
};

template <class K, class T> bool get(K a, T& b, Vec<pair<K, T>>& m) {
	for (auto ab: m)
		if (ab.first == a) {
			b = ab.second;
			return 1;
		}
	return 0;
}

template <class T> void dbgPrint(Vec<T>& v) {
	putchar('[');
	bool more = 0;
	for (auto& a: v) {
		if (more) dbgPrint(", ");
		more = 1;
		dbgPrint(a);
	}
	putchar(']');
}
