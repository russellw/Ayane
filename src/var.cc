#include "main.h"

Var* var(size_t i, LeafType* ty) {
	while (ty->vars.n <= i) {
		ty->vars.add(0);
		ty->alts.add(0);
	}
	if (!ty->vars[i]) {
		ty->vars[i] = new (ialloc(sizeof(Var))) Var(i, ty);
		ty->alts[i] = new (ialloc(sizeof(Var))) Var(i, ty);
	}
	return ty->vars[i];
}
