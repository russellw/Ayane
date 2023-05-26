// a predicate is a function with Boolean return type. A first-order constant (or propositional variable) is a function of arity
// zero.
struct Fn: Expr {
	Type* ty;
	char* s;

	Fn(Type* ty, char* s): Expr(Tag::fn), ty(ty), s(s) {
	}
};
