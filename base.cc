#include "main.h"
// TODO rename to etc?

// SORT
const char* basename(const char* file) {
	auto i = strlen(file);
	while (i) {
		switch(file[i - 1]){
			case'/':
#ifdef _WIN32
case'\\':
case':':
#endif
		 return file + i;
		}
		--i;
	}
	return file;
}

void err(const char* msg) {
	// TODO: different exit codes for different kinds of errors?
	if (parser::file) {
		size_t line = 1;
		for (auto s = parser::srco; s != parser::srck; ++s)
			if (*s == '\n') ++line;
		fprintf(stderr, "%s:%zu: %s\n", parser::file, line, msg);
	} else
		fprintf(stderr, "%s\n", msg);
	exit(1);
}

size_t fnv(const void* p, size_t bytes) {
	// Fowler-Noll-Vo-1a is slower than more sophisticated hash algorithms for large chunks of data, but faster for tiny ones, so it
	// still sees use
	auto q = (const unsigned char*)p;
	size_t h = 2166136261u;
	while (bytes--) {
		h ^= *q++;
		h *= 16777619;
	}
	return h;
}
///
