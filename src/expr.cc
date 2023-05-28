#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

// SORT
Expr* distinct(Vec<Expr*>& v) {
	Vec<Expr*> inequalities(v.n * (v.n - 1) / 2);
	size_t k = 0;
	for (auto i = v.begin(), e = v.end(); i < e; ++i)
		for (auto j = v.begin(); j < i; ++j) inequalities[k++] = comp(Tag::not1, comp(Tag::eq, *i, *j));
	return comp(Tag::and1, inequalities);
}

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

	Vec<Expr*> v(1 + vars.n, a);
	memcpy(v.data + 1, vars.data, vars.n * sizeof(void*));
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
	assert(size_t(a->tag) <= size_t(Tag::var));
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

		// If this expression is allocated in temporary buffer, all subexpressions which are likewise allocated, should be earlier
		// in the buffer
		if (inbuf(a))
			for (auto b: a)
				if (inbuf(b)) assert(b < a);

		// It is tempting to think we can add an 'else' to perform another check: If this expression is permanently allocated, all
		// subexpressions better be likewise. But that does not hold: Maybe the subexpressions are temporary, and this expression
		// should been likewise, but a particularly complex input formula caused temporary allocation to overflow the buffer and
		// spill into ialloc.

		// Recursively check subexpressions
		for (auto b: a) check(b);
		break;
	}
}

void dbgPrint(Tag tag) {
	static const char* tagNames[] = {
#define _(a) #a,
#include "tags.h"
	};
	dbgPrint(tagNames[(int)tag]);
}

void dbgPrint(Expr* a) {
	switch (a->tag) {
	case Tag::call:
		dbgPrint(at(a, 0));
		putchar('(');
		for (size_t i = 1; i < a->n; ++i) {
			if (i > 1) dbgPrint(", ");
			dbgPrint(at(a, i));
		}
		putchar(')');
		break;
	case Tag::fn:
	{
		auto s = ((Fn*)a)->s;
		if (s) dbgPrint(s);
		else
			printf("_%p", a);
		break;
	}
	case Tag::integer:
		mpz_out_str(stdout, 10, ((Int*)a)->v);
		break;
	case Tag::rat:
	case Tag::real:
		mpq_out_str(stdout, 10, ((Rat*)a)->v);
		break;
	case Tag::var:
		printf("%p", a);
		break;
	default:
		dbgPrint(a->tag);
		if (!a->n) break;
		putchar('(');
		for (size_t i = 0; i < a->n; ++i) {
			if (i) dbgPrint(", ");
			dbgPrint(at(a, i));
		}
		putchar(')');
		break;
	}
}
#endif
