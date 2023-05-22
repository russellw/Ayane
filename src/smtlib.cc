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
		case '+':
		case '-':
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
			auto s = src;
			while (isword[*(unsigned char*)s]) ++s;
			str = intern(src, s - src);
			src = s;
			tok = k_id;
			return;
		}
		case '#':
		{
			auto r = s + 2;
			char c;
			mpz_t z;
			switch (s[1]) {
			case 'b':
			{
				s = r;
				while (*s == '0' || *s == '1') ++s;

				// mpz_init_set_str doesn't like trailing junk, so give it a cleanly null-terminated string
				c = *s;
				*s = 0;

				// At least one digit is required
				if (mpz_init_set_str(z, r, 2)) err("Expected binary digit");
			}
			case 'x':
			{
				s = r;
				// TODO: unsigned char?
				while (isxdigit(*s)) ++s;

				// mpz_init_set_str doesn't like trailing junk, so give it a cleanly null-terminated string
				c = *s;
				*s = 0;

				// At least one digit is required
				if (mpz_init_set_str(z, r, 16)) err("Expected hex digit");
			}
			default:
				err("Stray '#'");
			}

			// The following byte might be important, so put it back
			*s = c;

			src = s;
			num = integer(z);
			tok = k_num;
			return;
		}
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
