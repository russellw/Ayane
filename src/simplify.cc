#include "main.h"

Expr* simplify(Expr* a) {
	// TODO: rename
	auto t = a->tag;
	// TODO: could this be folded into term creation?
	switch (t) {
	case Tag::isInteger:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return bools + isInteger(x);
		if (type(x) == &tinteger) return bools + 1;
		return expr(t, x);
	}
	case Tag::isRational:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return bools + 1;
		switch (type(x)->kind) {
		case Kind::integer:
		case Kind::rational:
			return bools + 1;
		}
		return expr(t, x);
	}
	case Tag::toInteger:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return toInteger(x);
		if (type(x) == &tinteger) return x;
		return expr(t, x);
	}
	case Tag::toRational:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return toRational(x);
		if (type(x) == &trational) return x;
		return expr(t, x);
	}
	case Tag::toReal:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return expr(ToReal, toRational(x));
		if (type(x) == &treal) return x;
		return expr(t, x);
	}
	}
	if (!a->n) return a;
	Vec<Expr*> v;
	for (size_t i = 0; i < a->n; ++i) v.add(simplify(at(a, i)));
	return expr(a->tag, v);
}
