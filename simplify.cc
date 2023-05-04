#include "all.h"

namespace {
bool constant(Ex* a) {
	switch (a->tag) {
	case DistinctObj:
	case Integer:
	case Rational:
	case True:
		return 1;
	case False:
		// In the superposition calculus, true only shows up as an argument of equality and false never shows up as an argument
		unreachable;
	}
	return 0;
}

bool realConstant(Ex* a) {
	return a->tag == ToReal && at(a, 0)->tag == Rational;
}
} // namespace

Ex* simplify(Ex* a) {
	// TODO: other simplifications e.g. x+0, x*1
	auto t = a->tag;
	// TODO: could this be folded into term creation?
	switch (t) {
	case Add:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return add(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, add(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case Ceil:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return ex(ToReal, ceil(at(x, 0)));
		if (constant(x)) return ceil(x);
		return ex(t, x);
	}
	case Div:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return div(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, div(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case DivE:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return divE(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, divE(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case DivF:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return divF(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, divF(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case DivT:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return divT(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, divT(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case Eq:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (x == y) return bools + 1;
		// TODO: optimize?
		if (constant(x) && constant(y)) return bools;
		if (realConstant(x) && realConstant(y)) return bools;
		return ex(t, x, y);
	}
	case Floor:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return ex(ToReal, floor(at(x, 0)));
		if (constant(x)) return floor(x);
		return ex(t, x);
	}
	case IsInteger:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return bools + isInteger(at(x, 0));
		if (constant(x)) return bools + isInteger(x);
		if (type(x) == &tinteger) return bools + 1;
		return ex(t, x);
	}
	case IsRational:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x) || constant(x)) return bools + 1;
		switch (type(x)->tag) {
		case Integer:
		case Rational:
			return bools + 1;
		}
		return ex(t, x);
	}
	case Le:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return bools + (cmp(x, y) <= 0);
		if (realConstant(x) && realConstant(y)) return bools + (cmp(at(x, 0), at(y, 0)) <= 0);
		return ex(t, x, y);
	}
	case Lt:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return bools + (cmp(x, y) < 0);
		if (realConstant(x) && realConstant(y)) return bools + (cmp(at(x, 0), at(y, 0)) < 0);
		return ex(t, x, y);
	}
	case Mul:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return mul(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, mul(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case Neg:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return ex(ToReal, neg(at(x, 0)));
		if (constant(x)) return neg(x);
		return ex(t, x);
	}
	case RemE:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return remE(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, remE(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case RemF:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return remF(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, remF(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case RemT:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return remT(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, remT(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case Round:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return ex(ToReal, round(at(x, 0)));
		if (constant(x)) return round(x);
		return ex(t, x);
	}
	case Sub:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return sub(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, sub(at(x, 0), at(y, 0)));
		return ex(t, x, y);
	}
	case ToInteger:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return toInteger(at(x, 0));
		if (constant(x)) return toInteger(x);
		if (type(x) == &tinteger) return x;
		return ex(t, x);
	}
	case ToRational:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return at(x, 0);
		if (constant(x)) return toRational(x);
		if (type(x) == &trational) return x;
		return ex(t, x);
	}
	case ToReal:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return x;
		if (constant(x)) return ex(ToReal, toRational(x));
		if (type(x) == &treal) return x;
		return ex(t, x);
	}
	case Trunc:
	{
		auto x = simplify(at(a, 0));
		if (realConstant(x)) return ex(ToReal, trunc(at(x, 0)));
		if (constant(x)) return trunc(x);
		return ex(t, x);
	}
	}
	if (!a->n) return a;
	vec<Ex*> v;
	for (size_t i = 0; i < a->n; ++i) v.add(simplify(at(a, i)));
	return ex(a->tag, v);
}
