Atom* intern(mpz_t a);
Atom* intern(mpq_t a);

// Functions for making arbitrary precision numbers for convenience, accept integer or string input and will intern the result so
// equality tests can simply compare pointers
Atom* integer(const char* s);

Atom* rational(const char* s);

// Real number literals are represented as rational number literals wrapped in ToReal. It's a function call that is not actually
// evaluated, since there is no representation of real number literals as such.
Term* real(mpq_t q);

// Per TPTP syntax, decimal/exponent string parses to a real number literal
Term* real(char* s);

// Arithmetic is polymorphic on integers and rationals
Atom* neg(Atom* a);
Atom* add(Atom* a, Atom* b);
Atom* sub(Atom* a, Atom* b);
Atom* mul(Atom* a, Atom* b);
Atom* div(Atom* a, Atom* b);

Atom* divT(Atom* a, Atom* b);
Atom* divF(Atom* a, Atom* b);
Atom* divE(Atom* a, Atom* b);
Atom* remT(Atom* a, Atom* b);
Atom* remF(Atom* a, Atom* b);
Atom* remE(Atom* a, Atom* b);

Atom* ceil(Atom* a);
Atom* floor(Atom* a);
Atom* trunc(Atom* a);
Atom* round(Atom* a);

// So is converting numbers between types
bool isInteger(Atom* a);

Atom* toInteger(Atom* a);
Atom* toRational(Atom* a);
Term* toReal(Atom* a);
