#ifdef _MSC_VER
// Not using exceptions
#pragma warning(disable : 4530)
#endif

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
#include "dbg.h"

#include "base.h"
#include "set.h"
#include "stats.h"
#include "vec.h"

// Logic
struct Expr;
struct Str;

#include "type.h"

#include "expr.h"

#include "clause.h"

// Parsers
#include "str.h"

#include "parser.h"

#include "dimacs.h"
#include "tptp.h"

// Algorithms
#include "cnf.h"
#include "etc.h"
#include "subsume.h"
#include "superposn.h"
#include "unify.h"
