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
#include <time.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>
// TODO: eliminate
using std::all_of;
using std::any_of;
using std::find;
using std::function;
using std::make_pair;
using std::max;
using std::min;
using std::move;
using std::none_of;
using std::pair;
using std::sort;
using std::swap;

#include <gmp.h>

// The debugging header needs to be included first, to make the assert macro available everywhere else, because assert is used all
// over the place, including in some inline functions defined in headers
#include "dbg.h"

// General
#include "base.h"
#include "stats.h"
#include "str.h"
#include "vec.h"

// Logic
#include "terms.h"

#include "bignums.h"
#include "clauses.h"

// I/O
#include "parser.h"

#include "dimacs.h"
#include "tptp.h"

// Algorithms
#include "cnf.h"
#include "etc.h"
#include "simplify.h"
#include "subsume.h"
#include "superposn.h"
#include "unify.h"
