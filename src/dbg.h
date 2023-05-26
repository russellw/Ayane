#ifdef DBG

// Print the value of an expression, along with where it is. Assumes a print function has been defined for the type.
#define dbg(a) \
	do { \
		printf("%s:%d: %s: %s: ", __FILE__, __LINE__, __func__, #a); \
		print(a); \
		putchar('\n'); \
	} while (0)

// Print stack trace. Intended for use in assert failure, but can also be used separately. Currently only implemented on Windows.
void stackTrace();

// Assert. Unlike the standard library macro, this prints a stack trace on Windows.
[[noreturn]] bool assertFail(const char* file, int line, const char* func, const char* s);
#define assert(a) (a) || assertFail(__FILE__, __LINE__, __func__, #a)
#define unreachable assert(0)

inline void print(char c) {
	putchar(c);
}

inline void print(int32_t n) {
	printf("%" PRId32, n);
}

inline void print(int64_t n) {
	printf("%" PRId64, n);
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

template <class K, class T> void print(const pair<K, T>& ab) {
	putchar('<');
	print(ab.first);
	print(", ");
	print(ab.second);
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

#else

#define dbg(a)

inline void stackTrace() {
}

#ifdef _MSC_VER
#define assert(a) __assume(a)
#define unreachable __assume(0)
#else
#define assert(a)
#define unreachable __builtin_unreachable()
#endif

#endif
