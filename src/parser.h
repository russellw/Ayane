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

enum
{
	k_id = 0x100,
	k_integer,
	k_rational,
	k_real,
	parser_k
};

struct parser {
	// Current file.
	static const char* file;

	// Heap allocation of source text.
	static uint32_t srco;
	uint32_t srcBytes;

	// Current position in source text.
	char* src;

	// Current token in source text.
	static char* srck;

	// Current token keyword or identifier.
	string* str;

	// Current token, as direct char for single-char tokens, or language-specific enum otherwise.
	int tok;

	// If we only needed to consider the happy path, all location variables, that keep track of which file we are parsing and where
	// we are in it, would just be instance variables. However we also need to consider errors, which may be detected in many
	// different functions that are not members of a parser class and have no access to its instance variables; those functions call
	// err() which therefore likewise has no access to parser instance variables. But err() needs to report file name and line
	// number, where applicable. Therefore, the location variables it needs, have been (above) declared static.

	// If we were only going to parse one file, that would be almost the end of the story. However, languages like TPTP allow a file
	// to include other files, which means we need to deal with a stack of locations. Instance variables handle that automatically,
	// but static variables do not. The solution is for the parser instance to store, for those particular location variables, not
	// the current values but the old values (presumably belonging to the file that included the current one) so they can be
	// restored by the destructor at end of file.

	// The special case where we are not in a file and err() does not need to report location, is covered automatically; at the end
	// of the top level file, the current file will be restored to its original value, which is null.
	const char* old_file;
	uint32_t old_srco;
	char* old_srck;

	parser(const char* file);
	~parser();

	// Lex an unquoted word, set symbol and set tok = k_id.
	void word();

	// Lex a quoted string, set symbol and leave tok unset.
	void quote();

	// Lex numbers; these functions just identify the end of a number token and set tok accordingly.
	void sign();
	void digits();
	void exp();
	void num();
};
