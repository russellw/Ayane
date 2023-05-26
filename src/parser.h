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

	// Current token in source text
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
	[[noreturn]] void err(const char* msg, int code = syntaxError);

	// Helper functions
	void digits();
	void lexInt(mpz_t z);
	void exponent(mpq_t q);

	// Lex a number, tok = k_num, num = the number
	void number();

	// Type a function, in context where error can report line number. For languages like TPTP where definitions can be implied on
	// the fly, but must be consistent with previous usage.
	Expr* fn(Str* s, Type* ty);

	// Check the correctness of expressions, in context where error can report line number
	void checkSize(Expr* a, size_t arity);
	void typing(Expr* a, Type* ty);
};
