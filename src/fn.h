// a predicate is a function with Boolean return type. A first-order constant (or propositional variable) is a function of arity
// zero.
struct Fn: Expr {
	char* s;
	Type* ty;

	Fn(char* s, Type* ty): Expr(Tag::fn), s(s), ty(ty) {
	}
};
