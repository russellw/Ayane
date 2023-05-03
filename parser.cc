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

void parser::digits() {
	auto s = src;
	while (isDigit(*s)) ++s;
	src = s;
}

void parser::integer(mpz_t z) {
	// mpz_init_set_str doesn't like leading '+', so eat it before proceeding
	if (*src == '+') ++src;

	auto s = src;
	if (*src == '-') ++src;
	digits();

	// mpz_init_set_str doesn't like trailing junk, so give it a cleanly null-terminated string
	auto c = *src;
	*src = 0;

	// At least one digit is required
	if (mpz_init_set_str(z, s, 10)) err("Expected number");

	// The following byte might be important, so put it back
	*src = c;
}

void parser::exponent(mpq_t q) {
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

void parser::num() {
	mpq_t q;
	tok = k_const;

	// Both TPTP and SMT-LIB require nonempty digit sequences before and after '.', which makes parsing slightly easier. Parsers
	// should only call this function if they detect at least one digit.
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
