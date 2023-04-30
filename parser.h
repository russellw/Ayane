// Compared to the versions in ctype.h, these functions generate shorter code, and have defined behavior for all input values. They
// are of course not for use on natural-language text, only for ASCII-based file formats. These versions are branch-heavy, but in
// one test, were measured at 6 CPU cycles per character, identical to an alternative algorithm with fewer branches.
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
	k_id = 0x100,
	k_integer,
	k_rational,
	k_real,
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
	string* str;

	// Current token, as direct char for single-char tokens, or language-specific enum otherwise
	int tok;

	parser(const char* file);
	~parser();

	// Lex an unquoted word, set symbol and set tok = k_id
	void word();

	// Lex a quoted string, set symbol and leave tok unset
	void quote();

	// Lex numbers; these functions just identify the end of a number token and set tok accordingly
	void sign();
	void digits();
	void exp();
	void num();

	// Report an error with line number, and exit
	[[noreturn]] void err(const char* msg);
};
