#include "main.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

const char* parser::file;
char* parser::srco;
char* parser::srck;

parser::parser(const char* file) {
	// Save the old location variables, so they can be restored when this file is done
	old_file = this->file;
	old_srco = srco;
	old_srck = srck;

	// And start assigning the new ones
	this->file = file;

	// Read all the input in one go before beginning parsing, to make the parsers simpler and faster. Testing indicates
	// memory-mapped files are not really faster for this case, so the input data is read-write so parsers can scribble over it if
	// that makes their job easier. The only thing parsers are not allowed change is the number of linefeed characters up to the
	// start of current token, because err() counts those linefeed characters to report line number.
	size_t n;
	size_t o;
	if (strcmp(file, "stdin")) {
#ifdef _WIN32
		_setmode(0, O_BINARY);
#endif
		n = 0;
		o = 0;
		const size_t block = 1 << 20;
		size_t cap = 0;
		for (;;) {
			if (n + block + 2 > cap) {
				auto cap1 = max(n + block + 2, cap * 2);
				o = heap->realloc(o, cap, cap1);
				cap = cap1;
			}
			auto r = read(0, (char*)heap->ptr(o) + n, block);
			if (r < 0) {
				perror(file);
				exit(1);
			}
			n += r;
			if (r != block) break;
		}
		srcBytes = cap;
	} else {
		auto f = open(file, O_BINARY | O_RDONLY);
		struct stat st;
		if (f < 0 || fstat(f, &st)) {
			perror(file);
			exit(1);
		}
		n = st.st_size;
		o = heap->alloc(n + 2);
		if (read(f, heap->ptr(o), n) != n) {
			perror(file);
			exit(1);
		}
		close(f);
		srcBytes = n + 2;
	}
	srco = o;

	// Make sure input is null terminated
	auto s = (char*)heap->ptr(o);
	s[n] = 0;

	// And ends in a newline, to simplify parser code
	if (!(n && s[n - 1] == '\n')) {
		s[n] = '\n';
		s[n + 1] = 0;
	}

	// Start at the beginning of the input
	src = s;

	// And set the current token likewise. Normally this won't matter, being overwritten as soon as the first token is lexed, but
	// some parser might have an edge case where it reports an error before doing so.
	srck = s;
}

parser::~parser() {
	free(src0);
}

void parser::err(const char* msg) {
	size_t line = 1;
	for (auto s = src0; s < srck; ++s)
		if (*s == '\n') ++line;
	fprintf(stderr, "%s:%zu: %s\n", file, line, msg);
	exit(1);
}

void parser::word() {
	auto s = src;
	while (isWord(*s)) ++s;
	str = intern(src, s - src);
	src = s;
	tok = k_id;
}

void parser::quote() {
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

bool parser::sign() {
	switch (*src) {
	case '+':
		++src;
		break;
	case '-':
		++src;
		return 1;
	}
	return 0;
}

void parser::digits() {
	auto s = src;
	do ++s;
	while (isDigit(*s));
	src = s;
}

void parser::exp() {
	switch (*src) {
	case 'E':
	case 'e':
		++src;
		sign();
		if (!isDigit(*src)) err("Expected digit");
		digits();
		break;
	}
}

void parser::num() {
	// mpz_init_set_str doesn't like leading '+', so eat it before proceeding
	if (*src == '+') {
		++src;
		srck = src;
	}

	mpq_t q;
	tok = k_const;

	// Result = scaled mantissa
	auto mantissa = mpq_numref(q);
	auto powScale = mpq_denref(q);

	// Integer. Even if it is followed by a decimal point, both TPTP and SMT-LIB require a nonempty integer part, which makes
	// parsing slightly easier. Parsers must only call this function if they detect at least one digit.
	assert(isDigit(*src) || *src == '-' && isDigit(src[1]));
	mpz_t z;
	auto s = src;
	do ++s;
	while (isDigit(*s));

	// mpz_init_set_str doesn't like trailing junk, so give it a cleanly null-terminated string
	auto c = *s;
	*s = 0;
	auto r = mpz_init_set_str(z, src, 10);

	// We have made sure to give mpz_init_set_str a correct number no matter what the input, so a parsing error at this point should
	// be impossible. If one happens anyway, that's an error in program logic.
	assert(!r);

	// The following byte might be important, so put it back
	*s = c;
	src = s;

	// After parsing the integer, we find out if this is actually a rational or decimal
	switch (c) {
	case '.':
		break;
	case '/':
		// It would be slightly faster to keep the integer we just parsed, but this case is so rare that it makes more sense to
		// optimize for simplicity and just let mpq_set_str do the whole job
		mpz_free(z);

		// Skip the '/' as well as the following digits
		digits();

		c = *src;
		*src = 0;

		mpq_init(q);
		if (mpq_set_str(q, srck, 10)) err("Invalid rational");
		mpq_canonicalize(q);

		*src = c;
		break;
	case 'E':
	case 'e':
		break;
	default:
		constant = ex(z);
		return;
	}
	constant = ex(q);

	// Decimal part
	size_t scale = 0;
	if (*src == '.') {
		++src;
		s = src;
		if (isDigit(*s)) {
			do ++s;
			while (isDigit(*s));
			auto c = *s;
			*s = 0;
			if (mpz_set_str(mantissa, src, 10)) err("Invalid decimal part");
			*s = c;
			scale = s - src;
			src = s;
		}
	}
	mpz_ui_pow_ui(powScale, 10, scale);

	// Mantissa += z * 10^scale
	mpz_addmul(mantissa, z, powScale);

	// Sign
	if (sign) mpz_neg(mantissa, mantissa);

	// Exponent
	bool exponentSign = 0;
	auto exponent = 0UL;
	if (*src == 'e' || *src == 'E') {
		++src;
		switch (*src) {
		case '-':
			exponentSign = 1;
			[[fallthrough]];
		case '+':
			++src;
			break;
		}
		errno = 0;
		exponent = strtoul(src, 0, 10);
		if (errno) err(strerror(errno));
	}
	mpz_t powExponent;
	mpz_init(powExponent);
	mpz_ui_pow_ui(powExponent, 10, exponent);
	if (exponentSign) mpz_mul(powScale, powScale, powExponent);
	else
		mpz_mul(mantissa, mantissa, powExponent);

	// Reduce result to lowest terms
	mpq_canonicalize(q);

	// Cleanup
	// TODO: free in reverse order?
	mpz_clear(powExponent);
	mpz_clear(z);

	// Wrap result in term designating it as a real number
	return real(q);
}
