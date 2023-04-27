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
	// the function is well-defined
	if (a->tag == Fn && a.size() > 1) {
		auto fty = a.getAtom()->ty;
		if (kind(fty) != kind::Fn) err("Called a non-function");
		check(a, fty.size() - 1);
		if (ty != fty[0]) err("Type mismatch");
		for (size_t i = 1; i < a->n; ++i) {
			switch (kind(fty[i])) {
			case kind::Bool:
			case kind::Fn:
				err("Invalid type for function argument");
			}
			check(a[i], fty[i]);
		}
		return;
	}

	// The core of the check: Make sure the term is of the required type
	if (type(a) != ty) err("Type mismatch");

	// Further checks can be done depending on operator. For example, arithmetic operators should have matching numeric arguments.
	switch (a->tag) {
	case Add:
	case DivE:
	case DivF:
	case DivT:
	case Mul:
	case RemE:
	case RemF:
	case RemT:
	case Sub:
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
		for (size_t i = 1; i < a->n; ++i) check(a[i], ty);
		return;
	case All:
	case Exists:
		check(a[1], kind::Bool);
		return;
	case And:
	case Eqv:
	case Not:
	case Or:
		for (size_t i = 1; i < a->n; ++i) check(a[i], kind::Bool);
		return;
	case Ceil:
	case Floor:
	case IsInteger:
	case IsRational:
	case Neg:
	case Round:
	case ToInteger:
	case ToRational:
	case ToReal:
	case Trunc:
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
		for (size_t i = 1; i < a->n; ++i) check(a[i], ty);
		return;
	case DistinctObj:
	case False:
	case Fn:
	case Integer:
	case Rational:
	case True:
		return;
	case Div:
		check(a, 2);
		ty = type(a[1]);
		switch (kind(ty)) {
		case kind::Rational:
		case kind::Real:
			break;
		default:
			err("Invalid type for rational division");
		}
		for (size_t i = 1; i < a->n; ++i) check(a[i], ty);
		return;
	case Eq:
		ty = type(a[1]);
		switch (kind(ty)) {
		case kind::Bool:
		case kind::Fn:
			err("Invalid type for equality");
		}
		check(a[1], ty);
		check(a[2], ty);
		return;
	case Le:
	case Lt:
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
	case Var:
		// A function would also be an invalid type for a variable, but we already checked for that
		if (kind(ty) == kind::Bool) err("Invalid type for variable");
		return;
	}
	unreachable;
}
