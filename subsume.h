bool subsumes(const clause& c, const clause& d);
bool subsumesForward(const set<clause>& cs, const clause& d);
set<clause> subsumeBackward(const set<clause>& cs, const clause& d);
