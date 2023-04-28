bool match(Term* a, Term* b, vec<pair<Term*, Term*>>& m);
bool unify(Term* a, bool ax, Term* b, bool bx, vec<pair<Termx, Termx>>& m);
Term* replace(Term* a, bool ax, const vec<pair<Termx, Termx>>& m);
