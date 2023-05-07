// Are clauses sets of literals, or bags? It would seem logical to represent them as sets, and some algorithms prefer it that way,
// but unfortunately there are important algorithms that will break unless they are represented as bags, such as the superposition
// calculus:
// https://stackoverflow.com/questions/29164610/why-are-clauses-multisets
// So we represent them as bags (or lists, ignoring the order) and let the algorithms that prefer sets, discard duplicate literals
struct Clause {
	uint16_t nn, n;
	bool dead;
	Ex* atoms[];

	range neg() const {
		return range(0, nn);
	}
	range pos() const {
		return range(nn, n);
	}
};

extern vec<Ex*> neg, pos;

inline Ex* at(Clause* c, size_t i) {
	assert(i < c->n);
	return c->atoms[i];
}

inline range::iterator begin(const Clause* c) {
	return 0;
}
inline range::iterator end(const Clause* c) {
	return c->n;
}

// Passive clauses are stored in a priority queue with smaller clauses first
size_t cost(Clause* c);

struct ClauseCompare {
	bool operator()(Clause* c, Clause* d) {
		return cost(c) > cost(d);
	}
};

extern priority_queue<Clause*, vector<Clause*>, ClauseCompare> passive;
void clause();
