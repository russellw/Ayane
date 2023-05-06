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

enum {
	k_const = 0x100,
	k_id,
	parser_k
};

struct parser {
	// Current file
	const char* file;

	// Source text
	char* src0;

	// Current position in source text
	char* src;

	// Current token in source text
	char* srck;

	// Current token keyword or identifier
	Str* str;

	// Current token, as direct char for single-char tokens, or language-specific enum otherwise
	int tok;

	// If tok == k_const, this is the value of the constant
	Ex* constant;

	parser(const char* file);
	~parser();

	// Report an error with line number, and exit
	[[noreturn]] void err(const char* msg);

	// Lex an unquoted word, set str, and set tok = k_id
	void word();

	// Lex a quoted string, set str, and leave tok unset, because different kinds of quotes can mean different things depending on
	// language
	void quote();

	// Helper functions
	void digits();
	void integer(mpz_t z);
	void exponent(mpq_t q);

	// Lex a number, set constant, and set tok = k_const
	void num();
};
