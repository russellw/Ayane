#include "main.h"

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
	return a->tag == ToReal && tag(at(a, 0)) == Rational;
}
} // namespace

Ex* simplify(const map<Ex*, Ex*>& m, Ex* a) {
	// TODO: other simplifications e.g. x+0, x*1
	auto t = a->tag;
	// TODO: could this be folded into term creation?
	switch (t) {
	case Add:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return x + y;
		if (realConstant(x) && realConstant(y)) return ex(ToReal, x[1] + y[1]);
		return ex(t, x, y);
	}
	case Ceil:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return ex(ToReal, ceil(x[1]));
		if (constant(x)) return ceil(x);
		return ex(t, x);
	}
	case DistinctObj:
	case False:
	case Integer:
	case Rational:
	case True:
		return a;
	case Div:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return x / y;
		if (realConstant(x) && realConstant(y)) return ex(ToReal, x[1] / y[1]);
		return ex(t, x, y);
	}
	case DivE:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return divE(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, divE(x[1], y[1]));
		return ex(t, x, y);
	}
	case DivF:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return divF(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, divF(x[1], y[1]));
		return ex(t, x, y);
	}
	case DivT:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return divT(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, divT(x[1], y[1]));
		return ex(t, x, y);
	}
	case Eq:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (x == y) return True;
		// TODO: optimize?
		if (constant(x) && constant(y)) return False;
		if (realConstant(x) && realConstant(y)) return False;
		return ex(t, x, y);
	}
	case Floor:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return ex(ToReal, floor(x[1]));
		if (constant(x)) return floor(x);
		return ex(t, x);
	}
	case Fn:
	{
		if (a.size() == 1) {
			// TODO: optimize
			if (m.count(a)) return m.at(a);
			return a;
		}
		vec<Ex*> v(1, a[0]);
		for (size_t i = 0; i < a->n; ++i) v.add(simplify(m, at(a, i)));
		return ex(v);
	}
	case IsInteger:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return tbool(isInteger(x[1]));
		if (constant(x)) return tbool(isInteger(x));
		if (type(x) == kind::Integer) return True;
		return ex(t, x);
	}
	case IsRational:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x) || constant(x)) return True;
		switch (kind(type(x))) {
		case kind::Integer:
		case kind::Rational:
			return True;
		}
		return ex(t, x);
	}
	case Le:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return tbool(x <= y);
		if (realConstant(x) && realConstant(y)) return tbool(x[1] <= y[1]);
		return ex(t, x, y);
	}
	case Lt:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return tbool(x < y);
		if (realConstant(x) && realConstant(y)) return tbool(x[1] < y[1]);
		return ex(t, x, y);
	}
	case Mul:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return x * y;
		if (realConstant(x) && realConstant(y)) return ex(ToReal, x[1] * y[1]);
		return ex(t, x, y);
	}
	case Neg:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return ex(ToReal, -x[1]);
		if (constant(x)) return -x;
		return ex(t, x);
	}
	case RemE:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return remE(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, remE(x[1], y[1]));
		return ex(t, x, y);
	}
	case RemF:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return remF(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, remF(x[1], y[1]));
		return ex(t, x, y);
	}
	case RemT:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return remT(x, y);
		if (realConstant(x) && realConstant(y)) return ex(ToReal, remT(x[1], y[1]));
		return ex(t, x, y);
	}
	case Round:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return ex(ToReal, round(x[1]));
		if (constant(x)) return round(x);
		return ex(t, x);
	}
	case Sub:
	{
		auto x = simplify(m, at(a, 0));
		auto y = simplify(m, at(a, 1));
		if (constant(x) && constant(y)) return x - y;
		if (realConstant(x) && realConstant(y)) return ex(ToReal, x[1] - y[1]);
		return ex(t, x, y);
	}
	case ToInteger:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return toInteger(x[1]);
		if (constant(x)) return toInteger(x);
		if (type(x) == kind::Integer) return x;
		return ex(t, x);
	}
	case ToRational:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return x[1];
		if (constant(x)) return toRational(x);
		if (type(x) == kind::Rational) return x;
		return ex(t, x);
	}
	case ToReal:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return x;
		if (constant(x)) return ex(ToReal, toRational(x));
		if (type(x) == kind::Real) return x;
		return ex(t, x);
	}
	case Trunc:
	{
		auto x = simplify(m, at(a, 0));
		if (realConstant(x)) return ex(ToReal, trunc(x[1]));
		if (constant(x)) return trunc(x);
		return ex(t, x);
	}
	case Var:
		if (m.count(a)) return m.at(a);
		return a;
	}
	unreachable;
}
