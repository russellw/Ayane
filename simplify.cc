#include "main.h"

namespace {
bool constant(Ex* a) {
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

Ex* simplify(Ex* a) {
	// TODO: other simplifications e.g. x+0, x*1
	// TODO: rename
	auto t = a->tag;
	// TODO: could this be folded into term creation?
	switch (t) {
	case Tag::add:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return add(x, y);
		return ex(t, x, y);
	}
	case Tag::ceil:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return ceil(x);
		return ex(t, x);
	}
	case Tag::div:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return div(x, y);
		return ex(t, x, y);
	}
	case Tag::dive:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return dive(x, y);
		return ex(t, x, y);
	}
	case Tag::divf:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return divf(x, y);
		return ex(t, x, y);
	}
	case Tag::divt:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return divt(x, y);
		return ex(t, x, y);
	}
	case Tag::eq:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (x == y) return bools + 1;
		// TODO: optimize?
		if (constant(x) && constant(y)) return bools;
		return ex(t, x, y);
	}
	case Tag::floor:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return floor(x);
		return ex(t, x);
	}
	case Tag::isInteger:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return bools + isInteger(x);
		if (type(x) == &tinteger) return bools + 1;
		return ex(t, x);
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
		return ex(t, x);
	}
	case Tag::le:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return bools + (cmp(x, y) <= 0);
		return ex(t, x, y);
	}
	case Tag::lt:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return bools + (cmp(x, y) < 0);
		return ex(t, x, y);
	}
	case Tag::minus:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return minus(x);
		return ex(t, x);
	}
	case Tag::mul:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return mul(x, y);
		return ex(t, x, y);
	}
	case Tag::reme:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return reme(x, y);
		return ex(t, x, y);
	}
	case Tag::remf:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return remf(x, y);
		return ex(t, x, y);
	}
	case Tag::remt:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return remt(x, y);
		return ex(t, x, y);
	}
	case Tag::round:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return round(x);
		return ex(t, x);
	}
	case Tag::sub:
	{
		auto x = simplify(at(a, 0));
		auto y = simplify(at(a, 1));
		if (constant(x) && constant(y)) return sub(x, y);
		return ex(t, x, y);
	}
	case Tag::toInteger:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return toInteger(x);
		if (type(x) == &tinteger) return x;
		return ex(t, x);
	}
	case Tag::toRational:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return toRational(x);
		if (type(x) == &trational) return x;
		return ex(t, x);
	}
	case Tag::toReal:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return ex(ToReal, toRational(x));
		if (type(x) == &treal) return x;
		return ex(t, x);
	}
	case Tag::trunc:
	{
		auto x = simplify(at(a, 0));
		if (constant(x)) return trunc(x);
		return ex(t, x);
	}
	}
	if (!a->n) return a;
	Vec<Ex*> v;
	for (size_t i = 0; i < a->n; ++i) v.add(simplify(at(a, i)));
	return ex(a->tag, v);
}
