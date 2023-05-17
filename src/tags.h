// True needs the lowest tag value, for ordering equations in the superposition calculus
_(true1)

// Expressions that occur in clauses during inference
// SORT
_(add)
_(call)
_(ceil)
_(distinctObj)
_(div)
_(divEuclid)
_(divFloor)
_(divTrunc)
_(eq)
_(floor)
_(fn)
_(integer)
_(isInt)
_(isRat)
_(lt)
_(minus)
_(mul)
_(rat)
_(real)
_(remEuclid)
_(remFloor)
_(remTrunc)
_(round)
_(sub)
_(toInt)
_(toRat)
_(toReal)
_(trunc)
_(var)
///

// Expressions that only occur in input formulas
// SORT
_(all)
_(and1)
_(eqv)
_(exists)
_(false1)
_(not1)
_(or1)
///

#undef _
