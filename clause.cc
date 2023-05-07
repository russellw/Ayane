#include "all.h"

static size_t cost(Ex* a) {
	size_t n = 1;
	for (size_t i = 0; i < a->n; ++i) n += cost(at(a, i));
	return n;
}

size_t cost(Clause* c) {
	size_t n = 0;
	for (size_t i = 0; i < c->n; ++i) n += cost(c->atoms[i]);
	return n;
}

priority_queue<Clause*, vector<Clause*>, ClauseCompare> passive;
