#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

// SORT
Expr* imp(Expr* a, Expr* b) {
	return comp(Tag::or1, comp(Tag::not1, a), b);
}

bool occurs(Expr* a, Expr* b) {
	if (a == b) return 1;
	for (auto bi: b)
		if (occurs(a, bi)) return 1;
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
		if (f->t->kind != Kind::fn) return 0;
		return at(f->t, 0);
	}
	case Tag::distinctObj:
		return &tindividual;
	case Tag::fn:
	{
		auto f = (Fn*)a;
		return f->t;
	}
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
		return ((Var*)a)->t;
	}
	return type(at(a, 0));
}
//

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

void dbgCheck(Expr* a) {
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
		return;
	}
	assert(a->n);

	// If this expression is allocated in temporary buffer, all subexpressions which are likewise allocated, should be earlier in
	// the buffer
	if (inbuf(a))
		for (auto b: a)
			if (inbuf(b)) assert(b < a);

	// It is tempting to think we can add an 'else' to perform another check: if this expression is permanently allocated, all
	// subexpressions better be likewise. But that does not hold: maybe the subexpressions are temporary, and this expression should
	// been likewise, but a particularly complex input formula caused temporary allocation to overflow the buffer and spill into
	// ialloc.

	// Recursively check subexpressions
	for (auto b: a) dbgCheck(b);
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
		putchar('(');
		dbgPrint(at(a, 0));
		for (size_t i = 0; i < a->n; ++i) {
			if (i) putchar(' ');
			dbgPrint(at(a, i));
		}
		putchar(')');
		return;
	case Tag::fn:
	{
		auto s = ((Fn*)a)->s;
		if (s) dbgPrint(s);
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
	if (a->n) {
		putchar('(');
		dbgPrint(a->tag);
		for (auto b: a) {
			putchar(' ');
			dbgPrint(b);
		}
		putchar(')');
		return;
	}
	dbgPrint(a->tag);
}
#endif
