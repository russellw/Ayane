#include "main.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

Parser::Parser(const char* file): file(file) {
	// Read all the input in one go before beginning parsing, to make the parsers simpler and faster. Testing indicates
	// memory-mapped files are not really faster for this case, so the input data is read-write so parsers can scribble over it if
	// that makes their job easier. The only thing parsers are not allowed change is the number of linefeed characters up to the
	// start of current token, because err() counts those linefeed characters to report line number.
	size_t n;
	if (strcmp(file, "stdin") == 0) {
#ifdef _WIN32
		_setmode(0, O_BINARY);
#endif
		// Reading from standard input, we don't know the total size up front, so read in blocks of fixed size until we have
		// everything, increasing the buffer capacity by a factor of 2 when necessary, to avoid spending too much time reallocating
		const size_t block = 1 << 20;
		n = 0;
		size_t cap = 0;
		src = 0;
		for (;;) {
			if (n + block + 2 > cap) {
				auto cap1 = max(n + block + 2, cap * 2);
				src = (char*)realloc(src, cap1);
				cap = cap1;
			}
			auto r = read(0, src + n, block);
			if (r < 0) {
				perror(file);
				exit(1);
			}
			n += r;
			if (r != block) break;
		}
	} else {
		// Reading from a file, we can find out the total size up front
		auto f = open(file, O_BINARY | O_RDONLY);
		struct stat st;
		if (f < 0 || fstat(f, &st)) {
			perror(file);
			exit(1);
		}
		n = st.st_size;
		src = (char*)malloc(n + 2);
		if (read(f, src, n) != n) {
			perror(file);
			exit(1);
		}
		close(f);
	}
	src0 = src;

	// Make sure input is null terminated and ends in a newline, to simplify parser code
	src[n] = '\n';
	src[n + 1] = 0;
}

Parser::~Parser() {
	free(src0);
}

void Parser::err(const char* msg) {
	size_t line = 1;
	for (auto s = src0; s < srck; ++s)
		if (*s == '\n') ++line;
	fprintf(stderr, "%s:%zu: %s\n", file, line, msg);
	exit(1);
}

void Parser::word() {
	auto s = src;
	while (isWord(*s)) ++s;
	str = intern(src, s - src);
	src = s;
	tok = k_id;
}

void Parser::quote() {
	auto r = src;
	auto s = src;
	auto q = *s++;
	while (*s != q) {
		if (*s == '\\') ++s;
		if (*s < ' ') err("Unclosed quote");
		*r++ = *s++;
	}
	str = intern(src, r - src);
	src = s + 1;
}

void Parser::digits() {
	auto s = src;
	while (isDigit(*s)) ++s;
	src = s;
}

void Parser::integer(mpz_t z) {
	auto s = src;
	switch (*s) {
	case '+':
		// mpz_init_set_str doesn't like leading '+', so eat it before proceeding
		++s;
		[[fallthrough]];
	case '-':
		++src;
		break;
	}
	digits();

	// mpz_init_set_str doesn't like trailing junk, so give it a cleanly null-terminated string
	auto c = *src;
	*src = 0;

	// At least one digit is required
	if (mpz_init_set_str(z, s, 10)) err("Expected number");

	// The following byte might be important, so put it back
	*src = c;
}

void Parser::exponent(mpq_t q) {
	assert(*src == 'e' || *src == 'E');
	++src;

	errno = 0;
	auto e = strtoll(src, &src, 10);
	if (errno) err(strerror(errno));

	mpz_t powExponent;
	mpz_init(powExponent);

	if (e >= 0) {
		mpz_ui_pow_ui(powExponent, 10, e);
		mpz_mul(mpq_numref(q), mpq_numref(q), powExponent);
	} else {
		mpz_ui_pow_ui(powExponent, 10, -e);
		mpz_mul(mpq_denref(q), mpq_denref(q), powExponent);
	}

	mpz_clear(powExponent);
}

void Parser::num() {
	mpq_t q;
	tok = k_num;

	// Both TPTP and SMT-LIB require nonempty digit sequences before and after '.', which makes parsing slightly easier.
	auto z = mpq_numref(q);
	integer(z);

	// After parsing the integer, we find out if this is actually a rational or decimal
	switch (*src) {
	case '.':
	{
		++src;

		// Need to parse the decimal part, but also track exactly how many digits it was written as; 1.23 != 1.023.
		auto s = src;

		// The integer parsing function would otherwise accept a sign here, but that would not make sense
		if (!isDigit(*s)) err("Expected digit");

		mpz_t decimal;
		integer(decimal);

		// Given 1.23, first convert to 100/100, to make room, so to speak, to add in the decimal part.
		auto scale = src - s;
		auto powScale = mpq_denref(q);
		mpz_init(powScale);
		mpz_ui_pow_ui(powScale, 10, scale);
		mpz_mul(z, z, powScale);
		mpz_clear(powScale);

		// Now convert to 123/100
		if (*srck == '-') mpz_sub(z, z, decimal);
		else
			mpz_add(z, z, decimal);

		// Exponent
		if (*src == 'e' || *src == 'E') exponent(q);
		break;
	}
	case '/':
		++src;
		integer(mpq_denref(q));
		break;
	case 'E':
	case 'e':
		mpz_init_set_ui(mpq_denref(q), 1);
		exponent(q);
		break;
	default:
		constant = ex(z);
		return;
	}
	mpq_canonicalize(q);
	constant = ex(q);
}

void Parser::setType(Ex* a, Type* ty) {
	assert(a->tag == Tag::fn);
	if (a->ty == ty) return;
	if (!a->ty) {
		a->ty = ty;
		return;
	}
	// TODO: do we also need a case for unknown assigned type?
	err("Type mismatch");
}

Ex* Parser::setType(Str* s, Type* ty) {
	if (s->fn) {
		auto a = s->fn;
		assert(a->s == s->v);
		setType(a, ty);
		return a;
	}
	auto a = (Ex*)malloc(offsetof(Ex, s) + sizeof(char*));
	a->tag = Tag::fn;
	a->s = s->v;
	a->ty = ty;
	return a;
}

void Parser::check(Ex* a, size_t n) {
	if (a->n == n) return;
	if (a->tag == Tag::call) --n;
	sprintf(buf, "Expected %zu args", n);
	// TODO: maybe should  return different exit codes depending whether the input file is definitely bad versus just not understood
	err(buf);
}

void Parser::check(Ex* a, Type* ty) {
	// All symbols used in a formula must have specified types by the time this check is run. Otherwise, there would be no way of
	// knowing whether the types they will be given in the future, would have passed the check.
	// TODO: can a type still be unspecified by the time we get this far?
	if (!ty) err("Unspecified type");

	// In first-order logic, a function cannot return a function, nor can a variable store one. (That would be higher-order logic.)
	// The code should be written so that neither the top-level callers nor the recursive calls, can ever ask for a function to be
	// returned.
	assert(ty->kind != Kind::fn);

	// Need to handle call before checking the type of this term, because the type of a call is only well-defined if the type of the
	// function is well-defined
	if (a->tag == Tag::call) {
		assert(a->n > 1);
		auto fty = at(a, 0)->ty;
		if (!fty) err("Unspecified type");

		// Check for input like a(b) where a is just a constant
		if (fty->kind != Kind::fn) err("Called a non-function");

		// Check for input like a(b) followed by a(b, c)
		check(a, fty->n);

		// The core of the check: Make sure the term is of the required type. At this point, could have rejoined the common logic,
		// but call is by far the most common composite expression, so might as well keep it going on the fast track.
		if (at(fty, 0) != ty) err("Type mismatch");

		// And recur, based on the parameter types
		for (size_t i = 1; i < fty->n; ++i) {
			// TODO: does this check on parameter types need to be repeated here?
			switch (at(fty, i)->kind) {
			case Kind::boolean:
			case Kind::fn:
				err("Invalid type for function parameter");
			}
			check(at(a, i), at(fty, i));
		}
		return;
	}

	// The core of the check: Make sure the term is of the required type
	if (type(a) != ty) err("Type mismatch");

	// Further checks can be done depending on operator. For example, arithmetic operators should have matching numeric arguments.
	// In each case, the first step is to check the number of arguments, where applicable and necessary, and the last is to
	// recursively check the argument expressions.
	switch (a->tag) {
	case Tag::add:
	case Tag::divEuclid:
	case Tag::divFloor:
	case Tag::divTrunc:
	case Tag::mul:
	case Tag::reme:
	case Tag::remf:
	case Tag::remt:
	case Tag::sub:
		check(a, 2);
		if (!isNum(ty)) err("Invalid type for arithmetic");
		check(at(a, 0), ty);
		check(at(a, 1), ty);
		return;
	case Tag::all:
	case Tag::exists:
		check(at(a, 0), &tbool);
		return;
	case Tag::and1:
	case Tag::eqv:
	case Tag::not1:
	case Tag::or1:
		for (size_t i = 0; i < a->n; ++i) check(at(a, i), &tbool);
		return;
	case Tag::ceil:
	case Tag::floor:
	case Tag::isInteger:
	case Tag::isRational:
	case Tag::minus:
	case Tag::round:
	case Tag::toInteger:
	case Tag::toRational:
	case Tag::toReal:
	case Tag::trunc:
		check(a, 1);
		ty = type(at(a, 0));
		if (!isNum(ty)) err("Invalid type for arithmetic");
		check(at(a, 0), ty);
		return;
	case Tag::distinctObj:
	case Tag::false1:
	case Tag::fn:
	case Tag::integer:
	case Tag::rational:
	case Tag::true1:
		assert(!a->n);
		return;
	case Tag::div:
		check(a, 2);
		switch (ty->kind) {
		case Kind::rational:
		case Kind::real:
			break;
		default:
			err("Invalid type for rational division");
		}
		check(at(a, 0), ty);
		check(at(a, 1), ty);
		return;
	case Tag::eq:
		ty = type(at(a, 0));
		switch (ty->kind) {
		case Kind::boolean:
		case Kind::fn:
			err("Invalid type for equality");
		}
		check(at(a, 0), ty);
		check(at(a, 1), ty);
		return;
	case Tag::le:
	case Tag::lt:
		check(a, 2);
		ty = type(at(a, 0));
		if (!isNum(ty)) err("Invalid type for comparison");
		check(at(a, 0), ty);
		check(at(a, 1), ty);
		return;
	case Tag::var:
		// A function would also be an invalid type for a variable, but we already checked for that
		// TODO: does this need to be dynamically checked here?
		if (ty == &tbool) err("Invalid type for variable");
		return;
	}
	unreachable;
}
