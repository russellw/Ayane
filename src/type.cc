#include "main.h"

LeafType tbool(Kind::boolean);
LeafType tindividual(Kind::individual);
LeafType tinteger(Kind::integer);
LeafType trat(Kind::rat);
LeafType treal(Kind::real);

bool isNum(Type* t) {
	switch (t->kind) {
	case Kind::integer:
	case Kind::rat:
	case Kind::real:
		return 1;
	default:
		return 0;
	}
}

#ifdef DBG
void dbgPrint(Kind kind) {
	static const char* kindNames[] = {
#define _(a) #a,
#include "kinds.h"
	};
	dbgPrint(kindNames[(int)kind]);
}

void dbgPrint(Type* t) {
	switch (t->kind) {
	case Kind::fn:
		dbgPrint(at(t, 0));
		putchar('(');
		for (size_t i = 1; i < t->n; ++i) {
			if (i > 1) dbgPrint(", ");
			dbgPrint(at(t, i));
		}
		putchar(')');
		break;
	case Kind::opaque:
		dbgPrint(((OpaqueType*)t)->s);
		break;
	default:
		dbgPrint(t->kind);
		break;
	}
}
#endif
