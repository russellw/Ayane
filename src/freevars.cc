#include "main.h"

static Vec<Expr*> boundv;

void freeVars(Expr* a, Vec<Expr*>& freev) {
	switch (a->tag) {
	case Tag::all:
	case Tag::exists:
	{
		auto o = boundv.n;
		boundv.add(begin(a) + 1, a->n - 1);
		freeVars(at(a, 0), freev);
		boundv.n = o;
		return;
	}
	case Tag::var:
		if (!boundv.has(a) && !freev.has(a)) freev.add(a);
		return;
	}
	for (auto b: a) freeVars(b, freev);
}
