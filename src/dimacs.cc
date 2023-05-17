#include "main.h"

namespace {
enum {
	k_zero = ntoks,
};

struct Parser1: Parser {
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
			if (!isdigit(s[1])) {
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

	// A variable in propositional logic is a constant (i.e. function of arity zero) in first-order logic.
	Expr* propVar() {
		auto a = fn(str, &tbool);
		lex();
		return a;
	}

	// Top level
	Parser1(const char* file): Parser(file) {
		lex();
		if (tok == 'p') {
			while (isspace(*src)) ++src;

			if (!(src[0] == 'c' && src[1] == 'n' && src[2] == 'f')) err("Expected 'cnf'");
			src += 3;
			lex();

			if (tok != k_id) err("Expected count");
			lex();

			if (tok != k_id) err("Expected count");
			lex();
		}
		for (;;) switch (tok) {
			case '-':
				lex();
				neg.add(propVar());
				break;
			case 0:
				if (neg.n + pos.n) clause();
				return;
			case k_id:
				pos.add(propVar());
				break;
			case k_zero:
				lex();
				clause();
				neg.n = pos.n = 0;
				break;
			default:
				err("Syntax error");
			}
	}
};
} // namespace

void dimacs(const char* file) {
	Parser1 parser(file);
}
