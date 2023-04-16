#include "main.h"

int bufi;
char buf[4096];

void bufAdd(char c) {
	// Leave a spare byte so a null terminator can be added if necessary.
	if (bufi >= sizeof buf - 1) err("Token too long");
	buf[bufi++] = c;
}

void bufCopy(const char* src, const char* end) {
	auto n = end - src;
	if (n >= sizeof buf) err("Token too long");
	memcpy(buf, src, n);
	buf[n] = 0;
}
