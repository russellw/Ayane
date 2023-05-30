// This, and its effective sub-enums in specific parsers, are not enum classes because enumerated tokens will be freely mixed with
// literal characters
enum {
	k_num = 0x100,
	k_word,
	ntoks
};

struct Parser {
	// Name of the file being parsed, or "stdin" for standard input
	const char* file;

	// Source text
	char* src0;

	// Beginning of current token in source text
	char* srck;

	// Current position in source text
	char* src;

	// Current token, as direct char for single-char tokens, or language-specific enum otherwise
	int tok;

	// Current token keyword or identifier
	Str* str;

	// If tok == k_num, this is the parsed number
	Expr* num;

	Parser(const char* file);
	~Parser();

	// Report an error with line number, and exit
	[[noreturn]] void err(const char* msg, int code = inputError);

	// Helper functions
	void digits();
	void lexInt(mpz_t z);

	// Type a function, in context where error can report line number. For languages like TPTP where definitions can be implied on
	// the fly, but must be consistent with previous usage. Also used by DIMACS for a simpler case.
	Expr* fn(Type* t, Str* s);

	// Recursively type-check an expression, in context where error can report line number. Assumes as precondition that the
	// expression is structurally sound, i.e. the correct class of C++ object is present everywhere, including some type being
	// assigned to every function. Checks for inconsistent types and wrong numbers of arguments.
	void check(size_t n, Expr* a);
	void check(Type* t, Expr* a);
};
