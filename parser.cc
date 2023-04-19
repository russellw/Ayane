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
uint32_t parser::srco;
char* parser::srck;

parser::parser(const char* file) {
	// Save the old location variables, so they can be restored when this file is done.
	old_file = this->file;
	old_srco = srco;
	old_srck = srck;

	// And start assigning the new ones.
	this->file = file;

	// Read all the input in one go before beginning parsing, to make the parsers simpler and faster. Testing indicates
	// memory-mapped files are not really faster for this case, so the input data is read-write so parsers can scribble over it if
	// that makes their job easier. The only thing parsers are not allowed change is the number of linefeed characters up to the
	// start of current token, because err() counts those linefeed characters to report line number.
	size_t n;
	size_t o;
	if (strcmp(file, "stdin")) {
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
	} else {
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
	}
	srco = o;

	// Make sure input is null terminated.
	auto s = (char*)heap->ptr(o);
	s[n] = 0;

	// And ends in a newline, to simplify parser code.
	if (!(n && s[n - 1] == '\n')) {
		s[n] = '\n';
		s[n + 1] = 0;
	}

	// Start at the beginning of the input.
	src = s;

	// And set the current token likewise. Normally this won't matter, being overwritten as soon as the first token is lexed, but
	// some parser might have an edge case where it reports an error before doing so.
	srck = s;
}

parser::~parser() {
	heap->free(srco, srcBytes);
	file = old_file;
	srco = old_srco;
	srck = old_srck;
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

void parser::sign() {
	switch (*src) {
	case '+':
	case '-':
		++src;
		break;
	}
}

void parser::digits() {
	auto s = src;
	while (isDigit(*s)) ++s;
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
	sign();

	// GMP doesn't handle unary +, so need to omit it from token.
	if (*srck == '+') ++srck;

	// Make sure the call wasn't triggered by a sign or decimal point that fails to be actually followed by a number.
	if (!isDigit(*src) && !(*src == '.' && isDigit(src[1]))) err("Expected digit");

	// Integer part.
	digits();

	// Followed by various possibilities for fractional part or exponent.
	switch (*src) {
	case '.':
		tok = k_real;
		++src;
		digits();
		exp();
		return;
	case '/':
		tok = k_rational;
		++src;
		if (!isDigit(*src)) err("Expected digit");
		digits();
		return;
	case 'E':
	case 'e':
		tok = k_real;
		exp();
		return;
	}

	// No, it was just an integer after all.
	tok = k_integer;
}
