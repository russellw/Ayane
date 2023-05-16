#include "main.h"

char buf[bufSize];
char* bufp;

void* balloc(size_t n) {
	n = roundUp(n, 8);
	if (buf + bufSize - bufp < n) return ialloc(n);
	auto r = bufp;
#ifdef DEBUG
	memset(r, 0xcc, n);
#endif
	bufp += n;
	return r;
}
