#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

// SORT
void flatten(Tag tag, Expr* a, vector<Expr*>& r) {
	if (a->tag == tag) {
		for (size_t i = 0; i < a->n; ++i) flatten(tag, at(a, i), r);
		return;
	}
	r.push_back(a);
}

Expr* imp(Expr* a, Expr* b) {
	return comp(Tag::or1, comp(Tag::not1, a), b);
}

bool occurs(Expr* a, Expr* b) {
	if (a == b) return 1;
	for (size_t i = 0; i < b->n; ++i)
		if (occurs(a, at(b, i))) return 1;
	return 0;
}

Expr* quantify(Expr* a) {
	Vec<Expr*> vars;
	freeVars(a, vars);

	if (!vars.n) return a;

	Vec<Expr*> v(1, a);
	// TODO: add all at once
	for (auto x: vars) v.add(x);
	return comp(Tag::all, v);
}

Type* type(Expr* a) {
	switch (a->tag) {
	case Tag::add:
	case Tag::ceil:
	case Tag::div:
	case Tag::divEuclid:
	case Tag::divFloor:
	case Tag::divTrunc:
	case Tag::floor:
	case Tag::minus:
	case Tag::mul:
	case Tag::remEuclid:
	case Tag::remFloor:
	case Tag::remTrunc:
	case Tag::round:
	case Tag::sub:
	case Tag::trunc:
		return type(at(a, 0));
	case Tag::all:
	case Tag::and1:
	case Tag::eq:
	case Tag::eqv:
	case Tag::exists:
	case Tag::false1:
	case Tag::isInt:
	case Tag::isRat:
	case Tag::lt:
	case Tag::not1:
	case Tag::or1:
	case Tag::true1:
		return &tbool;
	case Tag::call:
	{
		auto f = (Fn*)at(a, 0);
		assert(f->tag == Tag::fn);
		return at(f->ty, 0);
	}
	case Tag::distinctObj:
		return &tindividual;
	case Tag::fn:
		return ((Fn*)a)->ty;
	case Tag::integer:
	case Tag::toInt:
		return &tinteger;
	case Tag::rat:
	case Tag::toRat:
		return &trat;
	case Tag::real:
	case Tag::toReal:
		return &treal;
	case Tag::var:
		return ((Var*)a)->ty;
	}
	unreachable;
}
///

Eqn::Eqn(Expr* a) {
	if (a->tag == Tag::eq) {
		first = at(a, 0);
		second = at(a, 1);
		return;
	}
	first = a;
	second = bools + 1;
}

#ifdef DBG
static bool inbuf(void* p0) {
	auto p = (char*)p0;
	assert(!(bufp <= p && p < buf + bufSize));
	return buf <= p && p < bufp;
}

void check(Expr* a) {
	assert(size_t(a->tag) < size_t(Tag::COUNT));
	switch (a->tag) {
	case Tag::distinctObj:
	case Tag::false1:
	case Tag::fn:
	case Tag::integer:
	case Tag::rat:
	case Tag::real:
	case Tag::true1:
	case Tag::var:
		assert(!a->n);
		break;
	default:
		assert(a->n);

		// If this expression is allocated in temporary buffer, all composite subexpressions better have been allocated earlier in
		// buffer. Otherwise, assuming this expression is in the permanent hash table of shared composite expressions, all
		// subexpressions better also be permanently allocated.
		if (inbuf(a))
			for (auto b: a) {
				if (b->n) {
					assert(inbuf(b));
					assert(b < a);
				}
			}

		// It is tempting to think we can add an 'else' to perform the reverse check: If this expression is permanently allocated,
		// all subexpressions better be likewise. But that does not hold: Maybe the subexpressions are temporary, and this
		// expression should been likewise, but a particularly complex input formula caused temporary allocation to overflow the
		// buffer and spill into ialloc.

		// Recursively check subexpressions
		for (auto b: a) check(b);
		break;
	}
}

void print(Tag tag) {
	static const char* tagNames[] = {
#define _(a) #a,
#include "tags.h"
	};
	print(tagNames[(int)tag]);
}

void print(Expr* a) {
	switch (a->tag) {
	case Tag::call:
		print(at(a, 0));
		putchar('(');
		for (size_t i = 1; i < a->n; ++i) {
			if (i > 1) print(", ");
			print(at(a, i));
		}
		putchar(')');
		return;
	case Tag::fn:
	{
		auto s = ((Fn*)a)->s;
		if (s) print(s);
		else
			printf("_%p", a);
		return;
	}
	case Tag::integer:
		mpz_out_str(stdout, 10, ((Int*)a)->v);
		return;
	case Tag::rat:
	case Tag::real:
		mpq_out_str(stdout, 10, ((Rat*)a)->v);
		return;
	case Tag::var:
		printf("%p", a);
		return;
	}
	print(a->tag);
	if (!a->n) return;
	putchar('(');
	for (size_t i = 0; i < a->n; ++i) {
		if (i) print(", ");
		print(at(a, i));
	}
	putchar(')');
}
#endif
