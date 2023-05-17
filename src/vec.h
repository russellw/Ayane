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
	// TODO: simplify
	using reverse_iterator = std::reverse_iterator<T*>;

	uint32_t n;
	T* data = fixed;

	Vec(const Vec&) = delete;
	Vec& operator=(const Vec&) = delete;

	explicit Vec(size_t o = 0) {
		init(o);
	}

	explicit Vec(size_t o, T a) {
		init(o);
		*data = a;
	}

	~Vec() {
		if (data != fixed) free(data);
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

	// Etc
	T& operator[](size_t i) {
		assert(i < n);
		return begin()[i];
	}

	void add(T a) {
		reserve(n + 1);
		data[n++] = a;
	}

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
