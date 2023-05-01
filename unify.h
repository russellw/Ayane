bool match(Ex* a, Ex* b, vec<pair<Ex*, Ex*>>& m);
bool unify(Ex* a, bool ax, Ex* b, bool bx, vec<pair<termx, termx>>& m);
Ex* replace(Ex* a, bool ax, const vec<pair<termx, termx>>& m);
