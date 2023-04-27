// Names of the inference rules used by the program, for use in tracking inferences and printing proofs
enum class rule {
#define _(x) x,
#include "rules.h"
};

extern const char* ruleNames[];

inline void print(rule rl) {
	print(ruleNames[(int)rl]);
}

// The SZS ontologies for automated reasoning software, or at least, the subset thereof that is used here
enum class szs {
#define _(x) x,
#include "szs.h"
	end
};

extern const char* szsNames[];

inline void print(szs status) {
	print(szsNames[(int)status]);
}

// Equality can be represented in term form like any other binary operator, but there are also algorithms that need to pay
// particular attention to equations, e.g. in order to apply them in both directions, enough that it is worth having a specific type
// for them in such contexts.
using equation = pair<term*, term*>;

// A clause is a collection of negative literals, and a collection of positive literals. These cannot be sets, because the
// superposition calculus becomes incomplete if duplicate literals are disallowed. They would ideally be bags; for now, they are
// represented as lists.
using clause = pair<vec<term*>, vec<term*>>;

// There are many ways to represent false and true as clauses, but simplification makes canonical versions. Defining them as macros
// is not ideal, but they cannot be global constants because 'const' doesn't work with objects the way it does with small integers,
// and they cannot be global variables because those have to be initialized before the thread-local heap.
// TODO: reevaluate this. maybe they could be global variables declared here and defined in heap.cc?
#define falsec make_pair(vec<term*>(), vec<term*>())
#define truec make_pair(vec<term*>(), vec<term*>{True})

// Matching and unification must in the general case deal with two clauses which are assumed to have logically distinct variable
// names, but it is inefficient to provide physically distinct variable names for each clause, so we logically extend variable names
// with subscripts indicating which side they are on. Though the extended term type is meant only for variables, the type system is
// not fine-grained enough to make that distinction, so it is defined in terms of terms.
using termx = pair<term*, bool>;

// The component of the proof record that tracks, for each clause generated by CNF conversion, the formula from which it was derived
using ProofCnf = map<clause, term*>;

// Inference graph, or strictly speaking, hypergraph; each clause in a set of inferences is, in general, derived from multiple
// parent clauses. Note that this represents a single derivation of each clause from multiple parent clauses, not multiple
// derivations. It is possible and indeed common for a clause to have multiple separate derivations, that is, independent
// rediscoveries, but this is not particularly useful, so we eliminate the possibility of representing the rediscoveries.
using Proof = map<clause, pair<rule, vec<clause>>>;

// Type check
void check(term* a, type ty);
