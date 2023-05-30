#include "main.h"

Vec<Expr*> neg, pos;

static size_t cost(Expr* a) {
	size_t n = 1;
	for (auto b: a) n += cost(b);
	return n;
}

size_t cost(Clause* c) {
	size_t n = 0;
	// TODO: foreach?
	for (size_t i = 0; i < c->n; ++i) n += cost(c->v[i]);
	return n;
}

priority_queue<Clause*, vector<Clause*>, ClauseCompare> passive;

void clause() {
	for (auto a: neg) dbgCheck(a);
	for (auto a: pos) dbgCheck(a);

	// Normalize atoms
	initNorm();
	for (auto& a: neg) a = norm(a);
	for (auto& a: pos) a = norm(a);

	// Redundancy
	size_t i = 0;
	for (auto a: neg)
		if (a != bools + 1) neg[i++] = a;
	neg.n = i;

	i = 0;
	for (auto a: pos)
		if (a != bools) pos[i++] = a;
	pos.n = i;

	// Tautology
	if (neg.has(bools)) return;
	if (pos.has(bools + 1)) return;
	for (auto a: neg)
		if (pos.has(a)) return;

	// Make clause
	size_t nn = neg.n;
	size_t n = nn + pos.n;
	auto c = new (ialloc(offsetof(Clause, v) + n * sizeof(void*))) Clause(nn, n);
	memcpy(c->v, neg.data, nn * sizeof(void*));
	memcpy(c->v + nn, pos.data, pos.n * sizeof(void*));

	// Add to priority queue
	passive.push(c);
}

#ifdef DBG
void dbgPrintAtoms(Clause* c, Range r) {
	for (auto i: r) {
		if (i > r.first) dbgPrint(" | ");
		dbgPrint(at(c, i));
	}
}

void dbgPrint(Clause* c) {
	dbgPrintAtoms(c, c->neg());
	dbgPrint(" => ");
	dbgPrintAtoms(c, c->pos());
}
#endif
