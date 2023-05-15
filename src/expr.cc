#include "main.h"

Expr bools[2] = {{Tag::false1}, {Tag::true1}};

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

#ifdef DBG
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
