#include "main.h"

Var* var(size_t i, LeafType* ty) {
	if (i < ty->vars.n) return ty->vars[i];

	assert(i == ty->vars.n);
	auto a = new (ialloc(sizeof(Var))) Var(ty);
	ty->vars.add(a);
	return a;
}
