// TODO: separate file?
// TODO: change to use pointers?
struct Range: pair<size_t, size_t> {
	struct iterator {
		size_t i;

		iterator(size_t i): i(i) {
		}

		size_t operator*() {
			return i;
		}

		iterator& operator++() {
			++i;
			return *this;
		}

		bool operator!=(iterator x) {
			return i != x.i;
		}
	};

	Range() {
	}
	Range(size_t first, size_t second): pair(first, second) {
	}

	iterator begin() {
		return first;
	}
	iterator end() {
		return second;
	}
};

// SORT
size_t fnv(const void* p, size_t n);
///

// For debugging purposes, define print functions for all the data types being used
inline void print(char c) {
	putchar(c);
}

inline void print(int n) {
	printf("%d", n);
}

inline void print(uint32_t n) {
	printf("%" PRIu32, n);
}

inline void print(uint64_t n) {
	printf("%" PRIu64, n);
}

inline void print(const char* s) {
	printf("%s", s);
}

inline void print(const void* p) {
	printf("%p", p);
}

template <class K, class T> void print(const pair<K, T>& p) {
	putchar('<');
	print(p.first);
	print(", ");
	print(p.second);
	putchar('>');
}

template <class T> void print(const vector<T>& v) {
	putchar('[');
	bool more = 0;
	for (auto& a: v) {
		if (more) print(", ");
		more = 1;
		print(a);
	}
	putchar(']');
}

// SORT
inline size_t hashCombine(size_t a, size_t b) {
	return a ^ b + 0x9e3779b9u + (a << 6) + (a >> 2);
}

inline size_t roundUp(size_t n, size_t alignment) {
	return (n + alignment - 1) & ~(alignment - 1);
}
///

// Error codes specified by this program have numbers starting at 200 to avoid overlap with those specified by POSIX or Windows.
// Numbers are assigned roughly in order from 'things that can reasonably happen even when the input is not actually incorrect'
// (e.g. problem contains higher order logic that the system does not understand, 'inappropriate' in SZS terminology) to 'things
// that can only happen if there is an error in the input' (e.g. syntax error) to 'things that can only happen if there is a bug in
// the code' (e.g. assert failure).
const int inappropriateError = 200;
const int typeError = 201;
const int syntaxError = 202;
const int assertError = 203;

// SORT
void* ialloc(size_t n);
void mpz_ediv_q(mpz_t q, const mpz_t n, const mpz_t d);
void mpz_ediv_r(mpz_t r, const mpz_t n, const mpz_t d);
void mpz_round(mpz_t q, const mpz_t n, const mpz_t d);
///
