#ifdef DBG

// Print the value of an expression, along with where it is. Assumes a dbgPrint function has been defined for the type
#define dbg(a) \
	do { \
		printf("%s:%d: %s: %s: ", __FILE__, __LINE__, __func__, #a); \
		dbgPrint(a); \
		putchar('\n'); \
	} while (0)

// Print stack trace. Intended for use in assert failure, but can also be used separately. Currently only implemented on Windows
void stackTrace();

// Assert. Unlike the standard library macro, this prints a stack trace on Windows
[[noreturn]] bool assertFail(const char* file, int line, const char* func, const char* s);
#define assert(a) (a) || assertFail(__FILE__, __LINE__, __func__, #a)
#define unreachable assert(0)

//todo:best layout?
// SORT
inline void dbgPrint(char c) { putchar(c); }

inline void dbgPrint(const char* s) { printf("%s", s); }

inline void dbgPrint(const void* p) { printf("%p", p); }

inline void dbgPrint(int32_t n) { printf("%" PRId32, n); }

inline void dbgPrint(int64_t n) { printf("%" PRId64, n); }

inline void dbgPrint(uint32_t n) { printf("%" PRIu32, n); }

inline void dbgPrint(uint64_t n) { printf("%" PRIu64, n); }

template <class K, class T> void dbgPrint(const pair<K, T>& ab) {
	putchar('<');
	dbgPrint(ab.first);
	dbgPrint(", ");
	dbgPrint(ab.second);
	putchar('>');
}

template <class T> void dbgPrint(const vector<T>& v) {
	putchar('[');
	bool more = 0;
	for (auto& a: v) {
		if (more) dbgPrint(", ");
		more = 1;
		dbgPrint(a);
	}
	putchar(']');
}
//

#else

#define dbg(a)

inline void stackTrace() {}

#ifdef _MSC_VER
#define assert(a) __assume(a)
#define unreachable __assume(0)
#else
#define assert(a)
#define unreachable __builtin_unreachable()
#endif

#endif
