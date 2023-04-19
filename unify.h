bool match(map<term, term>& m, term a, term b);
bool unify(map<termx, termx>& m, term a, bool ax, term b, bool bx);
term replace(const map<termx, termx>& m, term a, bool ax);
