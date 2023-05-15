#include "main.h"

OpaqueType* opaqueType(Str* s) {
	if (s->ty) return s->ty;
	return s->ty = new (ialloc(sizeof(OpaqueType))) OpaqueType(s->v);
}
