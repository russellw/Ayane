#include "main.h"

Expr* var(size_t i, LeafType* ty) {
	// TODO: optimize
	while (ty->vars.n <= i) ty->vars.add(0);
	if (!ty->vars[i]) ty->vars[i] = new (ialloc(sizeof(Var))) Var(ty);
	return ty->vars[i];
}
