#include "main.h"

namespace {
bool constant(term a) {
	switch (tag(a)) {
	case tag::DistinctObj:
	case tag::Integer:
	case tag::Rational:
	case tag::True:
		return 1;
	case tag::False:
		// In the superposition calculus, true only shows up as an argument of equality and false never shows up as an argument.
		unreachable;
	}
	return 0;
}

bool realConstant(term a) {
	return tag(a) == tag::ToReal && tag(a[1]) == tag::Rational;
}
} // namespace

term simplify(const map<term, term>& m, term a) {
	// TODO: other simplifications e.g. x+0, x*1
	auto t = tag(a);
	switch (t) {
	case tag::Add:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return x + y;
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, x[1] + y[1]);
		return term(t, x, y);
	}
	case tag::Ceil:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(tag::ToReal, ceil(x[1]));
		if (constant(x)) return ceil(x);
		return term(t, x);
	}
	case tag::DistinctObj:
	case tag::False:
	case tag::Integer:
	case tag::Rational:
	case tag::True:
		return a;
	case tag::Div:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return x / y;
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, x[1] / y[1]);
		return term(t, x, y);
	}
	case tag::DivE:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return divE(x, y);
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, divE(x[1], y[1]));
		return term(t, x, y);
	}
	case tag::DivF:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return divF(x, y);
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, divF(x[1], y[1]));
		return term(t, x, y);
	}
	case tag::DivT:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return divT(x, y);
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, divT(x[1], y[1]));
		return term(t, x, y);
	}
	case tag::Eq:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (x == y) return tag::True;
		// TODO: optimize?
		if (constant(x) && constant(y)) return tag::False;
		if (realConstant(x) && realConstant(y)) return tag::False;
		return term(t, x, y);
	}
	case tag::Floor:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(tag::ToReal, floor(x[1]));
		if (constant(x)) return floor(x);
		return term(t, x);
	}
	case tag::Fn:
	{
		if (a.size() == 1) {
			// TODO: optimize
			if (m.count(a)) return m.at(a);
			return a;
		}
		vec<term> v(1, a[0]);
		for (size_t i = 1; i != a.size(); ++i) v.push_back(simplify(m, a[i]));
		return term(v);
	}
	case tag::IsInteger:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return tbool(isInteger(x[1]));
		if (constant(x)) return tbool(isInteger(x));
		if (type(x) == kind::Integer) return tag::True;
		return term(t, x);
	}
	case tag::IsRational:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x) || constant(x)) return tag::True;
		switch (kind(type(x))) {
		case kind::Integer:
		case kind::Rational:
			return tag::True;
		}
		return term(t, x);
	}
	case tag::Le:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return tbool(x <= y);
		if (realConstant(x) && realConstant(y)) return tbool(x[1] <= y[1]);
		return term(t, x, y);
	}
	case tag::Lt:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return tbool(x < y);
		if (realConstant(x) && realConstant(y)) return tbool(x[1] < y[1]);
		return term(t, x, y);
	}
	case tag::Mul:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return x * y;
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, x[1] * y[1]);
		return term(t, x, y);
	}
	case tag::Neg:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(tag::ToReal, -x[1]);
		if (constant(x)) return -x;
		return term(t, x);
	}
	case tag::RemE:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return remE(x, y);
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, remE(x[1], y[1]));
		return term(t, x, y);
	}
	case tag::RemF:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return remF(x, y);
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, remF(x[1], y[1]));
		return term(t, x, y);
	}
	case tag::RemT:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return remT(x, y);
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, remT(x[1], y[1]));
		return term(t, x, y);
	}
	case tag::Round:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(tag::ToReal, round(x[1]));
		if (constant(x)) return round(x);
		return term(t, x);
	}
	case tag::Sub:
	{
		auto x = simplify(m, a[1]);
		auto y = simplify(m, a[2]);
		if (constant(x) && constant(y)) return x - y;
		if (realConstant(x) && realConstant(y)) return term(tag::ToReal, x[1] - y[1]);
		return term(t, x, y);
	}
	case tag::ToInteger:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return toInteger(x[1]);
		if (constant(x)) return toInteger(x);
		if (type(x) == kind::Integer) return x;
		return term(t, x);
	}
	case tag::ToRational:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return x[1];
		if (constant(x)) return toRational(x);
		if (type(x) == kind::Rational) return x;
		return term(t, x);
	}
	case tag::ToReal:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return x;
		if (constant(x)) return term(tag::ToReal, toRational(x));
		if (type(x) == kind::Real) return x;
		return term(t, x);
	}
	case tag::Trunc:
	{
		auto x = simplify(m, a[1]);
		if (realConstant(x)) return term(tag::ToReal, trunc(x[1]));
		if (constant(x)) return trunc(x);
		return term(t, x);
	}
	case tag::Var:
		if (m.count(a)) return m.at(a);
		return a;
	}
	unreachable;
}

// TODO: normalize variables
clause simplify(const map<term, term>& m, const clause& c) {
	vec<term> neg;
	for (auto& a: c.first) {
		auto b = simplify(m, a);
		switch (tag(b)) {
		case tag::False:
			return truec;
		case tag::True:
			continue;
		}
		neg.push_back(b);
	}

	vec<term> pos;
	for (auto& a: c.second) {
		auto b = simplify(m, a);
		switch (tag(b)) {
		case tag::False:
			continue;
		case tag::True:
			return truec;
		}
		pos.push_back(b);
	}

	for (auto& a: neg)
		if (find(pos.begin(), pos.end(), a) != pos.end()) return truec;

	return make_pair(neg, pos);
}

set<clause> simplify(const map<term, term>& m, const set<clause>& cs) {
	set<clause> r;
	for (auto& c0: cs) {
		auto c = simplify(m, c0);
		if (c == truec) continue;
		r.add(c);
	}
	return r;
}
