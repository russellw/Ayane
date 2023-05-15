#include "main.h"

static Vec<Expr*> boundv;

void freeVars(Expr* a, Vec<Expr*>& freev) {
	switch (a->tag) {
	case Tag::all:
	case Tag::exists:
	{
		auto o = boundv.n;
		// TODO: batch add
		for (size_t i = 1; i < a->n; ++i) boundv.add(at(a, i));
		freeVars(at(a, 0), freev);
		boundv.n = o;
		return;
	}
	case Tag::var:
		if (!boundv.has(a) && !freev.has(a)) freev.add(a);
		return;
	}
	for (size_t i = 0; i < a->n; ++i) freeVars(at(a, i), freev);
}
