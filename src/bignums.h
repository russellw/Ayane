Expr* intern(mpz_t a);
Expr* intern(mpq_t a);

// Real number literals are represented as rational number literals wrapped in ToReal. It's a function call that is not actually
// evaluated, since there is no representation of real number literals as such.
// TODO: reevaluate this; we have room for a Real tag anyway
Expr* real(mpq_t q);

// Per TPTP syntax, decimal/exponent string parses to a real number literal
Expr* real(char* s);

// Arithmetic is polymorphic on integers and rationals
Expr* minus(Expr* a);
Expr* add(Expr* a, Expr* b);
Expr* sub(Expr* a, Expr* b);
Expr* mul(Expr* a, Expr* b);
Expr* div(Expr* a, Expr* b);

Expr* divTrunc(Expr* a, Expr* b);
Expr* divFloor(Expr* a, Expr* b);
Expr* divEuclid(Expr* a, Expr* b);
Expr* remTrunc(Expr* a, Expr* b);
Expr* remFloor(Expr* a, Expr* b);
Expr* remEuclid(Expr* a, Expr* b);

Expr* ceil(Expr* a);
Expr* floor(Expr* a);
Expr* trunc(Expr* a);
Expr* round(Expr* a);

// So is converting numbers between types
bool isInteger(Expr* a);

Expr* toInteger(Expr* a);
Expr* toRational(Expr* a);
Expr* toReal(Expr* a);
