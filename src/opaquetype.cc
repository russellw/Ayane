#include "main.h"

// TODO: is this used by anything except TPTP?
OpaqueType* opaqueType(Str* s) {
	if (s->ty) {
		assert(s->ty->kind == Kind::opaque);
		return (OpaqueType*)s->ty;
	}
	auto ty = new (ialloc(sizeof(OpaqueType))) OpaqueType(s->v);
	s->ty = ty;
	return ty;
}
