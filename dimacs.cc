#include "main.h"

namespace {
enum {
	k_zero = parser_k,
};

struct parser1: parser {
	// Tokenizer
	void lex() {
	loop:
		auto s = srck = src;
		switch (*s) {
		case ' ':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '\v':
			src = s + 1;
			goto loop;
		case '0':
			if (!isDigit(s[1])) {
				src = s + 1;
				tok = k_zero;
				return;
			}
			[[fallthrough]];
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			word();
			return;
		case 'c':
			src = strchr(s, '\n');
			goto loop;
		case 0:
			tok = 0;
			return;
		}
		src = s + 1;
		tok = *s;
	}

	// A variable in propositional logic is a constant in first-order logic
	Atom* var() {
		auto a = term(str, kind::Bool);
		lex();
		return a;
	}

	// Top level
	void add(const vec<Term*>& literals) {
		cnf(new Formula(file, 0, term(literals)));
	}

	parser1(const char* file): parser(file) {
		lex();
		if (tok == 'p') {
			while (isSpace(*src)) ++src;

			if (!(src[0] == 'c' && src[1] == 'n' && src[2] == 'f')) err("Expected 'cnf'");
			src += 3;
			lex();

			if (tok != k_id) err("Expected count");
			lex();

			if (tok != k_id) err("Expected count");
			lex();
		}
		vec<Term*> literals(1, term(Or));
		for (;;) switch (tok) {
			case '-':
				lex();
				literals.add(term(Not, var()));
				break;
			case 0:
				if (literals.n > 1) add(literals);
				return;
			case k_id:
				literals.add(var());
				break;
			case k_zero:
				lex();
				add(literals);
				literals.n = 1;
				break;
			default:
				err("Syntax error");
			}
	}
};
} // namespace

void parseDimacs(const char* file) {
	parser1 _(file);
}
