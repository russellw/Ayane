// Compared to the versions in ctype.h, these functions generate shorter code, and have defined behavior for all input values. They
// are of course not for use on natural-language text, only for ASCII-based file formats. These versions are branch-heavy, but in
// one test, were measured at 6 CPU cycles per character, identical to an alternative algorithm with fewer branches.
// TODO: test speed against current standard library implementations
inline bool isSpace(int c) {
	return 0 < c && c <= ' ';
}

inline bool isPrint(int c) {
	return ' ' < c && c < 127;
}

inline bool isUpper(int c) {
	return 'A' <= c && c <= 'Z';
}

inline bool isLower(int c) {
	return 'a' <= c && c <= 'z';
}

inline bool isAlpha(int c) {
	return isLower(c) || isUpper(c);
}

inline bool isDigit(int c) {
	return '0' <= c && c <= '9';
}

inline bool isAlnum(int c) {
	return isAlpha(c) || isDigit(c);
}

inline bool isWord(int c) {
	return isAlnum(c) || c == '_';
}

// This, and its effective sub-enums in specific parsers, are not enum classes because enumerated tokens will be freely mixed with
// literal characters
enum {
	k_id = 0x100,
	k_num,
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
	[[noreturn]] void err(const char* msg);

	// Lex an unquoted word, tok = k_id, str = the word
	void word();

	// Lex a quoted string, leave tok unset (because different kinds of quotes can mean different things depending on language), str
	// = the string, minus quotes and escapes
	void quote();

	// Helper functions
	void digits();
	void integer(mpz_t z);
	void exponent(mpq_t q);

	// Lex a number, tok = k_num, num = the number
	void number();

	// Type a function, in context where error can report line number
	void setType(Fn* a, Type* ty);
	Expr* fn(Str* s, Type* ty);

	// Check the correctness of expressions, in context where error can report line number
	void check(Expr* a, size_t arity);
	void check(Expr* a, Type* ty);
};
