#include "main.h"

LeafType tbool(Kind::boolean);
LeafType tindividual(Kind::individual);
LeafType tinteger(Kind::integer);
LeafType trat(Kind::rat);
LeafType treal(Kind::real);

bool isNum(Type* ty) {
	switch (ty->kind) {
	case Kind::integer:
	case Kind::rat:
	case Kind::real:
		return 1;
	default:
		return 0;
	}
}

#ifdef DBG
void print(Kind kind) {
	static const char* kindNames[] = {
#define _(a) #a,
#include "kinds.h"
	};
	print(kindNames[(int)kind]);
}

void print(Type* ty) {
	switch (ty->kind) {
	case Kind::fn:
		print(at(ty, 0));
		putchar('(');
		for (size_t i = 1; i < ty->n; ++i) {
			if (i > 1) print(", ");
			print(at(ty, i));
		}
		putchar(')');
		break;
	case Kind::opaque:
		print(((OpaqueType*)ty)->s);
		break;
	default:
		print(ty->kind);
		break;
	}
}
#endif
