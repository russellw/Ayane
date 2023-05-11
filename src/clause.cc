#include "main.h"

Vec<Expr*> neg, pos;

static size_t cost(Expr* a) {
	size_t n = 1;
	for (size_t i = 0; i < a->n; ++i) n += cost(at(a, i));
	return n;
}

size_t cost(Clause* c) {
	size_t n = 0;
	for (size_t i = 0; i < c->n; ++i) n += cost(c->v[i]);
	return n;
}

priority_queue<Clause*, vector<Clause*>, ClauseCompare> passive;

void clause() {
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
	for (auto a: neg)
		if (a == bools) return;
	for (auto a: pos)
		if (a == bools + 1) return;
	for (auto a: neg)
		for (auto b: pos)
			if (a == b) return;

	// Make clause
	size_t nn = neg.n;
	size_t n = nn + pos.n;
	auto c = (Clause*)malloc(offsetof(Clause, v) + n * sizeof(Expr*));
	c->nn = nn;
	c->n = n;
	c->dead = 0;
	memcpy(c->v, neg.data, nn * sizeof(Expr*));
	memcpy(c->v + nn, pos.data, pos.n * sizeof(Expr*));

	// Add to priority queue
	passive.push(c);
}
