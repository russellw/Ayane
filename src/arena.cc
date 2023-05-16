#include "main.h"

char arena[arenaSize];
static char* p;

// SORT
void* aalloc(size_t n) {
	n = roundUp(n, 8);
	if (arena + arenaSize - p < n) return ialloc(n);
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
