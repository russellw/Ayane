// Printing things that need to be joined with separators.
#define joining bool joining1 = 0
#define join(s) \
	do { \
		if (joining1) print(s); \
		joining1 = 1; \
	} while (0)

// SORT
const char* basename(const char* file);
[[noreturn]] void err(const char* msg);
size_t fnv(const void* p, size_t bytes);
///

// For debugging purposes, define print functions for all the data types being used.
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

template <class T> void print(const std::vector<T>& v) {
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
inline size_t divUp(size_t n, size_t alignment) {
	return (n + alignment - 1) / alignment;
}

inline size_t hashCombine(size_t a, size_t b) {
	return a ^ b + 0x9e3779b9u + (a << 6) + (a >> 2);
}

constexpr bool isPow2(size_t n) {
	assert(n);
	return !(n & n - 1);
}

inline size_t roundUp(size_t n, size_t alignment) {
	return (n + alignment - 1) & ~(alignment - 1);
}
///

// Set and map containers are based on hash tables, so in general we need to be able to hash everything. The standard library uses a
// more complex protocol based on 'template <> struct hash<...>' classes in namespace std, but since we have homebrew containers,
// there is no particular requirement to follow the standard library protocol.
inline size_t hash(const void* p) {
	return fnv(&p, sizeof p);
}

inline size_t hash(size_t n) {
	return n;
}

template <class T, class U> size_t hash(const pair<T, U>& p) {
	return hashCombine(hash(p.first), hash(p.second));
}
