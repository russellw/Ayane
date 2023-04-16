template <class T> using graph = set<pair<T, T>>;

// SORT
template <class T> void dfs(const graph<T>& g, const T& a, function<void(const T&)> f) {
	set<T> visited;
	dfs(g, a, f, visited);
}

template <class T> void dfs(const graph<T>& g, const T& a, function<void(const T&)> f, set<T>& visited) {
	if (!visited.add(a)) return;
	f(a);
	for (auto& b: successors(g, a)) dfs(g, b, f, visited);
}

template <class T> void dfsWithout(const graph<T>& g, const T& a, const T& w, function<void(const T&)> f) {
	set<T> visited;
	dfsWithout(g, a, w, f, visited);
}

template <class T> void dfsWithout(const graph<T>& g, const T& a, const T& w, function<void(const T&)> f, set<T>& visited) {
	if (a == w) return;
	if (!visited.add(a)) return;
	f(a);
	for (auto& b: successors(g, a)) dfsWithout(g, b, w, f, visited);
}

template <class T> set<T> domFrontier(const graph<T>& g, const T& s, const T& a) {
	set<T> r;
	for (auto& b: nodes(g)) {
		if (strictlyDominates(g, s, a, b)) continue;
		for (auto& c: predecessors(g, b))
			if (dominates(g, s, a, c)) {
				r.add(b);
				break;
			}
	}
	return r;
}

template <class T> bool dominates(const graph<T>& g, const T& s, const T& a, const T& b) {
	return !reachesWithout(g, s, b, a);
}

template <class T> T idom(const graph<T>& g, const T& s, const T& b) {
	for (auto& a: nodes(g))
		if (isIdom(g, s, a, b)) return a;
	return T();
}

template <class T> bool isIdom(const graph<T>& g, const T& s, const T& a, const T& b) {
	if (!strictlyDominates(g, s, a, b)) return 0;
	for (auto& c: strictDominators(g, s, b))
		if (!dominates(g, s, c, a)) return 0;
	return 1;
}

template <class T> set<T> nodes(const graph<T>& g) {
	set<T> r;
	for (auto& p: g) {
		r.add(p.first);
		r.add(p.second);
	}
	return r;
}

template <class T> set<T> predecessors(const graph<T>& g, const T& a) {
	set<T> r;
	for (auto& p: g)
		if (p.second == a) r.add(p.first);
	return r;
}

template <class T> bool reaches(const graph<T>& g, const T& a, const T& b) {
	bool r = 0;
	dfs(g, a, [&](const T& c) {
		if (c == b) r = 1;
	});
	return r;
}

template <class T> bool reachesWithout(const graph<T>& g, const T& a, const T& b, const T& w) {
	bool r = 0;
	dfsWithout<T>(g, a, w, [&](const T& c) {
		if (c == b) r = 1;
	});
	return r;
}

template <class T> set<T> strictDominators(const graph<T>& g, const T& s, const T& b) {
	set<T> r;
	for (auto& a: nodes(g))
		if (strictlyDominates(g, s, a, b)) r.add(a);
	return r;
}

template <class T> bool strictlyDominates(const graph<T>& g, const T& s, const T& a, const T& b) {
	return a != b && dominates(g, s, a, b);
}

template <class T> set<T> successors(const graph<T>& g, const T& a) {
	set<T> r;
	for (auto& p: g)
		if (p.first == a) r.add(p.second);
	return r;
}

template <class T> set<T> transSuccessors(const graph<T>& g, const T& a) {
	set<T> r;
	dfs<T>(g, a, [&](const T& b) { r.add(b); });
	r.erase(a);
	return r;
}
///
