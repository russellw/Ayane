Ex* intern(mpz_t a);
Ex* intern(mpq_t a);

// Real number literals are represented as rational number literals wrapped in ToReal. It's a function call that is not actually
// evaluated, since there is no representation of real number literals as such.
// TODO: reevaluate this; we have room for a Real tag anyway
Ex* real(mpq_t q);

// Per TPTP syntax, decimal/exponent string parses to a real number literal
Ex* real(char* s);

// Arithmetic is polymorphic on integers and rationals
Ex* minus(Ex* a);
Ex* add(Ex* a, Ex* b);
Ex* sub(Ex* a, Ex* b);
Ex* mul(Ex* a, Ex* b);
Ex* div(Ex* a, Ex* b);

Ex* divT(Ex* a, Ex* b);
Ex* divF(Ex* a, Ex* b);
Ex* divE(Ex* a, Ex* b);
Ex* remT(Ex* a, Ex* b);
Ex* remF(Ex* a, Ex* b);
Ex* remE(Ex* a, Ex* b);

Ex* ceil(Ex* a);
Ex* floor(Ex* a);
Ex* trunc(Ex* a);
Ex* round(Ex* a);

// So is converting numbers between types
bool isInteger(Ex* a);

Ex* toInteger(Ex* a);
Ex* toRational(Ex* a);
Ex* toReal(Ex* a);
