#include "main.h"

Var* var(LeafType* t, size_t i) {
	while (t->vars.n <= i) {
		t->vars.add(0);
		t->alts.add(0);
	}
	if (!t->vars[i]) {
		t->vars[i] = new (ialloc(sizeof(Var))) Var(t, i);
		t->alts[i] = new (ialloc(sizeof(Var))) Var(t, i);
	}
	return t->vars[i];
}
