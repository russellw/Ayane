#include "main.h"

namespace {
// SORT
#define end (arena + sizeof arena)
char arena[1000000];
char* p;
///
} // namespace

// SORT
void* aalloc(size_t n) {
	n = roundUp(n, 8);
	if (end - p < n) return ialloc(n);
	auto r = p;
#ifdef DEBUG
	memset(r, 0xcc, n);
#endif
	p += n;
	return r;
}

void aclear() {
	p = arena;
}
///
