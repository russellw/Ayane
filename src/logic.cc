#include "main.h"

const char* ruleNames[] = {
#define _(x) #x,
#include "rules.h"
};

const char* szsNames[] = {
#define _(x) #x,
#include "szs.h"
};

static void check(term a, size_t arity) {
	if (a.size() - 1 == arity) return;
	sprintf(buf, "Expected %zu args, received %zu", arity, a.size() - 1);
	err(buf);
}

void check(term a, type ty) {
	// In first-order logic, a function cannot return a function, nor can a variable store one. (That would be higher-order logic.)
	// The code should be written so that neither the top-level callers nor the recursive calls, can ever ask for a function to be
	// returned.
	assert(kind(ty) != kind::Fn);

	// All symbols used in a formula must have specified types by the time this check is run. Otherwise, there would be no way of
	// knowing whether the types they will be given in the future, would have passed the check.
	if (ty == kind::Unknown) err("Unspecified type");

	// Need to handle calls before checking the type of this term, because the type of a call is only well-defined if the type of
	// the function is well-defined.
	if (tag(a) == tag::Fn && a.size() > 1) {
		auto fty = a.getAtom()->ty;
		if (kind(fty) != kind::Fn) err("Called a non-function");
		check(a, fty.size() - 1);
		if (ty != fty[0]) err("Type mismatch");
		for (size_t i = 1; i != a.size(); ++i) {
			switch (kind(fty[i])) {
			case kind::Bool:
			case kind::Fn:
				err("Invalid type for function argument");
			}
			check(a[i], fty[i]);
		}
		return;
	}

	// The core of the check: Make sure the term is of the required type.
	if (type(a) != ty) err("Type mismatch");

	// Further checks can be done depending on operator. For example, arithmetic operators should have matching numeric arguments.
	switch (tag(a)) {
	case tag::Add:
	case tag::DivE:
	case tag::DivF:
	case tag::DivT:
	case tag::Mul:
	case tag::RemE:
	case tag::RemF:
	case tag::RemT:
	case tag::Sub:
		check(a, 2);
		ty = type(a[1]);
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for arithmetic");
		}
		for (size_t i = 1; i != a.size(); ++i) check(a[i], ty);
		return;
	case tag::All:
	case tag::Exists:
		check(a[1], kind::Bool);
		return;
	case tag::And:
	case tag::Eqv:
	case tag::Not:
	case tag::Or:
		for (size_t i = 1; i != a.size(); ++i) check(a[i], kind::Bool);
		return;
	case tag::Ceil:
	case tag::Floor:
	case tag::IsInteger:
	case tag::IsRational:
	case tag::Neg:
	case tag::Round:
	case tag::ToInteger:
	case tag::ToRational:
	case tag::ToReal:
	case tag::Trunc:
		check(a, 1);
		ty = type(a[1]);
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for arithmetic");
		}
		for (size_t i = 1; i != a.size(); ++i) check(a[i], ty);
		return;
	case tag::DistinctObj:
	case tag::False:
	case tag::Fn:
	case tag::Integer:
	case tag::Rational:
	case tag::True:
		return;
	case tag::Div:
		check(a, 2);
		ty = type(a[1]);
		switch (kind(ty)) {
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for rational division");
		}
		for (size_t i = 1; i != a.size(); ++i) check(a[i], ty);
		return;
	case tag::Eq:
		ty = type(a[1]);
		switch (kind(ty)) {
		case kind::Bool:
		case kind::Fn:
			err("Invalid type for equality");
		}
		check(a[1], ty);
		check(a[2], ty);
		return;
	case tag::Le:
	case tag::Lt:
		check(a, 2);
		ty = type(a[1]);
		switch (kind(ty)) {
		case kind::Integer:
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for comparison");
		}
		check(a[1], ty);
		check(a[2], ty);
		return;
	case tag::Var:
		// A function would also be an invalid type for a variable, but we already checked for that.
		if (kind(ty) == kind::Bool) err("Invalid type for variable");
		return;
	}
	unreachable;
}
