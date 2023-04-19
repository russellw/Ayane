#include "main.h"

// SORT
const char* basename(const char* file) {
	auto i = strlen(file);
	while (i) {
		if (file[i - 1] == '/') return file + i;
#ifdef _WIN32
		if (file[i - 1] == '\\') return file + i;
#endif
		--i;
	}
	return file;
}

void err(const char* msg) {
	if (parser::file) {
		size_t line = 1;
		for (auto s = (char*)heap->ptr(parser::srco); s != parser::srck; ++s)
			if (*s == '\n') ++line;
		fprintf(stderr, "%s:%zu: %s\n", parser::file, line, msg);
	} else
		fprintf(stderr, "%s\n", msg);
	exit(1);
}

size_t fnv(const void* p, size_t bytes) {
	// Fowler-Noll-Vo-1a is slower than more sophisticated hash algorithms for large chunks of data, but faster for tiny ones, so it
	// still sees use.
	auto q = (const unsigned char*)p;
	size_t h = 2166136261u;
	while (bytes--) {
		h ^= *q++;
		h *= 16777619;
	}
	return h;
}
///
