#include "main.h"

Var* var(LeafType* ty, size_t i) {
	while (ty->vars.n <= i) {
		ty->vars.add(0);
		ty->alts.add(0);
	}
	if (!ty->vars[i]) {
		ty->vars[i] = new (ialloc(sizeof(Var))) Var(ty, i);
		ty->alts[i] = new (ialloc(sizeof(Var))) Var(ty, i);
	}
	return ty->vars[i];
}
