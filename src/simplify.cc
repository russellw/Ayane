#include "main.h"

namespace {
bool constant(Expr* a) {
	switch (a->tag) {
	case Tag::distinctObj:
	case Tag::integer:
	case Tag::rational:
	case Tag::true1:
		return 1;
	case Tag::false1:
		// In the superposition calculus, true only shows up as an argument of equality and false never shows up as an argument
		unreachable;
	}
	return 0;
}
} // namespace

Expr* simplify(Expr* a) {
	// TODO: other simplifications e.g. x+0, x*1
	// TODO: rename
	auto t = a->tag;
	// TODO: could this be folded into term creation?
	switch (t) {
	case Tag::ceil:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return ceil(x);
		return expr(t, x);
	}
	case Tag::div:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return div(x, y);
		return expr(t, x, y);
	}
	case Tag::divEuclid:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return divEuclid(x, y);
		return expr(t, x, y);
	}
	case Tag::divFloor:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return divFloor(x, y);
		return expr(t, x, y);
	}
	case Tag::divTrunc:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return divTrunc(x, y);
		return expr(t, x, y);
	}
	case Tag::eq:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (x == y) return bools + 1;
		// TODO: optimize?
		if (constant(x) && constant(y)) return bools;
		return expr(t, x, y);
	}
	case Tag::floor:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return floor(x);
		return expr(t, x);
	}
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
	case Tag::le:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return bools + (cmp(x, y) <= 0);
		return expr(t, x, y);
	}
	case Tag::lt:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return bools + (cmp(x, y) < 0);
		return expr(t, x, y);
	}
	case Tag::minus:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return minus(x);
		return expr(t, x);
	}
	case Tag::remEuclid:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return remEuclid(x, y);
		return expr(t, x, y);
	}
	case Tag::remFloor:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return remFloor(x, y);
		return expr(t, x, y);
	}
	case Tag::remTrunc:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return remTrunc(x, y);
		return expr(t, x, y);
	}
	case Tag::round:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return round(x);
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
	case Tag::trunc:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return trunc(x);
		return expr(t, x);
	}
	}
	if (!a->n) return a;
	Vec<Expr*> v;
	for (size_t i = 0; i < a->n; ++i) v.add(simplify(at(a, i)));
	return expr(a->tag, v);
}
