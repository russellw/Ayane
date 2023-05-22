#ifdef _MSC_VER
// Not using exceptions
#pragma warning(disable : 4530)
#endif

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::priority_queue;
using std::sort;
using std::unordered_map;
using std::unordered_set;
using std::vector;

#include <gmp.h>

// Clang-format sorts headers in a block, which is good. However, some headers define things needed by others that would come before
// them in alphabetical order, so they are placed in separate blocks separated by blank lines.
#include "buf.h"
#include "dbg.h"
#include "etc.h"
#include "range.h"
#include "stats.h"
#include "vec.h"

#include "cartproduct.h"

// Types
struct Str;
struct Var;
#include "type.h"

#include "comptype.h"
#include "opaquetype.h"

// Expressions
#include "expr.h"

#include "comp.h"
#include "fn.h"
#include "int.h"
#include "rat.h"
#include "var.h"

#include "freevars.h"
#include "norm.h"
#include "unify.h"

// Clauses
#include "clause.h"
#include "cnf.h"
#include "subsume.h"

// Parsers
#include "str.h"

#include "parser.h"

#include "dimacs.h"
#include "smtlib.h"
#include "tptp.h"

// Solvers
#include "superposn.h"
