bool subsumes(Clause* c, Clause* d);
bool subsumesForward(const set<clause>& cs, Clause* d);
set<clause> subsumeBackward(const set<clause>& cs, Clause* d);
