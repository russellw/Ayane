#ifdef DEBUG

// Trace function entry and exit.
struct Tracer {
	Tracer();
	~Tracer();
};

// Use this by simply declaring 'trace;' typically at the start of a function.
#define trace Tracer tracer

// Indent according to trace level. Intended for use in the dbg macro, but can also be used separately.
void indent();

// Print the value of an expression, along with where it is, at an indentation level set by Tracer if used. Assumes a print function
// has been defined for the type.
#define dbg(a) \
	do { \
		indent(); \
		printf("%s:%d: %s: %s: ", __FILE__, __LINE__, __func__, #a); \
		print(a); \
		putchar('\n'); \
	} while (0)

// Print stack trace. Intended for use in assert failure, but can also be used separately. Currently only implemented on Windows.
void stackTrace();

// Assert; unlike the standard library one, this one prints a stack trace.
[[noreturn]] bool assertFail(const char* file, int line, const char* func, const char* s);
#define assert(a) (a) || assertFail(__FILE__, __LINE__, __func__, #a)
#define unreachable assert(0)

#else

#define trace
#define indent()
#define dbg(a)

#define stackTrace()
#ifdef _MSC_VER
#define assert(a) __assume(a)
#define unreachable __assume(0)
#else
#define assert(a)
#define unreachable __builtin_unreachable()
#endif

#endif
