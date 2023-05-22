#include "main.h"

namespace {
enum {
	k_zero = ntoks,
};

char isword[0x100];

struct Parser1: Parser {
	// Tokenizer
	void lex() {
	loop:
		auto s = srck = src;
		switch (*s) {
		case ' ':
		case '\n':
		case '\r':
		case '\t':
			src = s + 1;
			goto loop;
		case '!':
		case '$':
		case '%':
		case '&':
		case '*':
		case '.':
		case '/':
		case '<':
		case '=':
		case '>':
		case '?':
		case '@':
		case '^':
		case '_':
		case 'A':
		case 'a':
		case 'B':
		case 'b':
		case 'C':
		case 'c':
		case 'D':
		case 'd':
		case 'E':
		case 'e':
		case 'F':
		case 'f':
		case 'G':
		case 'g':
		case 'H':
		case 'h':
		case 'I':
		case 'i':
		case 'J':
		case 'j':
		case 'K':
		case 'k':
		case 'L':
		case 'l':
		case 'M':
		case 'm':
		case 'N':
		case 'n':
		case 'O':
		case 'o':
		case 'P':
		case 'p':
		case 'Q':
		case 'q':
		case 'R':
		case 'r':
		case 'S':
		case 's':
		case 'T':
		case 't':
		case 'U':
		case 'u':
		case 'V':
		case 'v':
		case 'W':
		case 'w':
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case 'Z':
		case 'z':
		case '~':
		{
			// TODO: the order of cases is weird
			// TODO: .9 allowed?
			auto s = src;
			while (isword[*(unsigned char*)s]) ++s;
			str = intern(src, s - src);
			src = s;
			tok = k_id;
			return;
		}
		case '+':
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			// TODO: +, - also word
			number();
			return;
		case ';':
			src = strchr(s, '\n');
			goto loop;
		}
		src = s + 1;
		tok = *s;
	}

	// Parser
	bool eat(int k) {
		if (tok == k) {
			lex();
			return 1;
		}
		return 0;
	}

	void expect(char k) {
		if (eat(k)) return;
		sprintf(buf, "Expected '%c'", k);
		err(buf);
	}

	// Top level
	Parser1(const char* file): Parser(file) {
		lex();
		while (tok) { expect('('); }
	}
};
} // namespace

void smtlib(const char* file) {
	isword['!'] = 1;
	isword['$'] = 1;
	isword['%'] = 1;
	isword['&'] = 1;
	isword['*'] = 1;
	isword['+'] = 1;
	isword['-'] = 1;
	isword['.'] = 1;
	isword['/'] = 1;
	isword['<'] = 1;
	isword['='] = 1;
	isword['>'] = 1;
	isword['?'] = 1;
	isword['@'] = 1;
	isword['^'] = 1;
	isword['_'] = 1;
	isword['~'] = 1;
	memset(isword + '0', 1, 10);
	memset(isword + 'A', 1, 26);
	memset(isword + 'a', 1, 26);
	Parser1 parser(file);
}
