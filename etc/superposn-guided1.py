# the #! line has been intentionally omitted here
# as this script requires subprocess capture_output
# which does not work with the version of /usr/bin/python3 in WSL at this time
# run this script with 'python ...'
import subprocess
import argparse
import datetime
import fractions
import heapq
import inspect
import itertools
import logging
import os
import re
import sys
import time


# naming conventions:
# C, D  clauses
# F, G  formulas
# a, b  terms or any values
# c     character
# e     exception
# f     function
# i, j  indexes
# m     dict (aka map)
# n     integer
# o     operator
# r     result
# s     collection, string
# t     type
# v     collection of variables
# x, y  variables, terms or any values

# numbers larger than 2000 silently fail
sys.setrecursionlimit(2000)

# make sure the bag representation compares by value
a = {"a": 1, "b": 2, "c": 3, "d": 4}
b = {"d": 4, "c": 3, "b": 2, "a": 1}
assert a == b

a = set(["a", "b", "c", "d", "e"])
b = set(["e", "b", "d", "c", "a"])
assert a == b


def check_tuples(a):
    if isinstance(a, tuple):
        for b in a:
            check_tuples(b)
        return
    if isinstance(a, list):
        raise ValueError(a)


def invert(m):
    r = dict(map(reversed, m.items()))
    assert len(r) == len(m)
    return r


def remove(s, i):
    s = list(s)
    del s[i]
    return tuple(s)


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


######################################## logging


logger = logging.getLogger()
logger.addHandler(logging.StreamHandler(sys.stdout))
logger.setLevel(logging.DEBUG)
pr_buf = ""


def pr(a):
    global pr_buf
    pr_buf += str(a)


def prn(a=""):
    global pr_buf
    logger.info(pr_buf + str(a))
    pr_buf = ""


def debug(a):
    logger.debug(str(a), stack_info=True)


######################################## limits


class MemoryOut(Exception):
    def __init__(self):
        super().__init__("MemoryOut")


class Timeout(Exception):
    def __init__(self):
        super().__init__("Timeout")


def set_timeout(seconds=3600):
    global end_time
    end_time = time.time() + seconds


def check_limits():
    if time.time() > end_time:
        raise Timeout()


######################################## terms


class DistinctObject:
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return self.name


distinct_objects = {}


def distinct_object(name):
    if name in distinct_objects:
        return distinct_objects[name]
    a = DistinctObject(name)
    distinct_objects[name] = a
    return a


# real number constants must be rational, but separate type from Fraction
class Real(fractions.Fraction):
    pass


# constants are just functions of arity zero
skolem_name_i = 0


class Fn:
    def __init__(self, name=None):
        global skolem_name_i
        if name is None:
            skolem_name_i += 1
            name = f"sk{skolem_name_i}"
        else:
            m = re.match(r"sK(\d+)", name)
            if m:
                skolem_name_i = max(skolem_name_i, int(m[1]))
        self.name = name

    def type_args(self, rt, args):
        self.t = rt
        if args:
            self.t = (rt,) + tuple([typeof(a) for a in args])

    def __repr__(self):
        return self.name


__fns = {}


def clear_fns():
    global skolem_name_i
    __fns.clear()
    skolem_name_i = 0


def fn(name):
    if name in __fns:
        return __fns[name]
    a = Fn(name)
    __fns[name] = a
    return a


# named types are handled as functions
types = {}


def mktype(name):
    if name in types:
        return types[name]
    a = Fn(name)
    types[name] = a
    return a


# first-order variables cannot be boolean
class Var:
    def __init__(self, t=None):
        if not t:
            return
        assert t != "bool"
        self.t = t

    def __repr__(self):
        if not hasattr(self, "name"):
            return "Var"
        return self.name


def const(a):
    if isinstance(a, bool):
        return True
    if isinstance(a, DistinctObject):
        return True
    if isinstance(a, int):
        return True
    if isinstance(a, fractions.Fraction):
        return True


def equatable(a, b):
    t = typeof(a)
    if t != typeof(b):
        return
    # normally first-order logic doesn't allow equality on predicates
    # superposition calculus makes a special exception
    # for the pseudo-equation p=true
    if t == "bool":
        return b is True
    return True


def equation(a):
    if isinstance(a, tuple) and a[0] == "=":
        return a
    return "=", a, True


def equation_atom(a, b):
    if b is True:
        return a
    return "=", a, b


def fns(a):
    s = set()

    def get_fn(a):
        if isinstance(a, Fn):
            s.add(a)

    walk(get_fn, a)
    return s


def free_vars(a):
    bound = set()
    free = []

    def get_free_vars(a, bound, free):
        if isinstance(a, tuple):
            if a[0] in ("exists", "forall"):
                bound = bound.copy()
                for x in a[1]:
                    bound.add(x)
                get_free_vars(a[2], bound, free)
                return
            for b in a[1:]:
                get_free_vars(b, bound, free)
            return
        if isinstance(a, Var):
            if a not in bound and a not in free:
                free.append(a)
            return

    get_free_vars(a, bound, free)
    return free


def imp(a, b):
    return "or", ("not", a), b


def match(a, b, m):
    if typeof(a) != typeof(b):
        return
    if isinstance(a, tuple) and isinstance(b, tuple):
        if len(a) != len(b):
            return
        if a[0] != b[0]:
            return
        for i in range(1, len(a)):
            if not match(a[i], b[i], m):
                return
        return True
    if a == b:
        return True
    if isinstance(a, Var):
        if a in m:
            return m[a] == b
        m[a] = b
        return True


def occurs(a, b, m):
    assert isinstance(a, Var)
    if a is b:
        return True
    if isinstance(b, tuple):
        for bi in b:
            if occurs(a, bi, m):
                return True
    if b in m:
        return occurs(a, m[b], m)


def quantify(a):
    v = free_vars(a)
    if v:
        return "forall", v, a
    return a


def rename_vars(a, m):
    if isinstance(a, tuple):
        return tuple([rename_vars(b, m) for b in a])
    if isinstance(a, Var):
        if a in m:
            return m[a]
        b = Var(a.t)
        m[a] = b
        return b
    return a


def simplify(a):
    if isinstance(a, tuple):
        a = tuple(map(simplify, a))
        o = a[0]
        if o in ("+", "-", "/", "*"):
            x = a[1]
            y = a[2]
            if const(x) and const(y):
                t = typeof(x)
                a = eval(f"x{o}y")
                if t == "real":
                    return Real(a)
            return a
        if o in ("<", "<="):
            x = a[1]
            y = a[2]
            if const(x) and const(y):
                t = typeof(x)
                return eval(f"x{o}y")
            return a
        if o == "=":
            x = a[1]
            y = a[2]
            if x == y:
                return True
            if unequal(x, y):
                return False
            return a
    return a


def splice(a, path, b, i=0):
    if i == len(path):
        return b
    a = list(a)
    j = path[i]
    a[j] = splice(a[j], path, b, i + 1)
    return tuple(a)


def subst(a, m):
    if a in m:
        return subst(m[a], m)
    if isinstance(a, tuple):
        return tuple([subst(b, m) for b in a])
    return a


def term_size(a):
    if isinstance(a, tuple):
        n = 0
        for b in a:
            n += term_size(b)
        return n
    return 1


def unequal(a, b):
    if const(a) and const(b):
        return a != b


def unify(a, b, m):
    if typeof(a) != typeof(b):
        return
    if isinstance(a, tuple) and isinstance(b, tuple):
        if len(a) != len(b):
            return
        if a[0] != b[0]:
            return
        for i in range(1, len(a)):
            if not unify(a[i], b[i], m):
                return
        return True
    if a == b:
        return True

    def unify_var(a, b, m):
        if a in m:
            return unify(m[a], b, m)
        if b in m:
            return unify(a, m[b], m)
        if occurs(a, b, m):
            return
        m[a] = b
        return True

    if isinstance(a, Var):
        return unify_var(a, b, m)
    if isinstance(b, Var):
        return unify_var(b, a, m)


def unquantify(a):
    if isinstance(a, tuple) and a[0] == "forall":
        return a[2]
    return a


def walk(f, a):
    if isinstance(a, tuple):
        for b in a:
            walk(f, b)
    f(a)


######################################## types


def typeof(a):
    if isinstance(a, tuple):
        o = a[0]
        if isinstance(o, str):
            if o in (
                "exists",
                "forall",
                "eqv",
                "=",
                "<",
                "<=",
                "and",
                "or",
                "int?",
                "rat?",
            ):
                return "bool"
            if o.startswith("to-"):
                return o[3:]
            return typeof(a[1])
        t = typeof(o)
        if not isinstance(t, tuple):
            raise ValueError(a)
        return t[0]
    if isinstance(a, Fn) or isinstance(a, Var):
        return a.t
    if isinstance(a, bool):
        return "bool"
    if isinstance(a, DistinctObject):
        return "individual"
    if isinstance(a, int):
        return "int"
    # as subclass, Real must be checked before Fraction
    if isinstance(a, Real):
        return "real"
    if isinstance(a, fractions.Fraction):
        return "rat"
    raise ValueError(a)


# first step of type inference:
# unify to figure out how all the unspecified types can be made consistent
def type_unify(wanted, a, m):
    # this version of unify skips the type check
    # because it makes no sense to ask the type of a type
    def unify(a, b, m):
        if isinstance(a, tuple) and isinstance(b, tuple):
            if len(a) != len(b):
                return
            for i in range(len(a)):
                if not unify(a[i], b[i], m):
                    return
            return True
        if a == b:
            return True

        def unify_var(a, b, m):
            if a in m:
                return unify(m[a], b, m)
            if b in m:
                return unify(a, m[b], m)
            if occurs(a, b, m):
                return
            m[a] = b
            return True

        if isinstance(a, Var):
            return unify_var(a, b, m)
        if isinstance(b, Var):
            return unify_var(b, a, m)

    if not unify(wanted, typeof(a), m):
        raise ValueError(f"{wanted} != typeof({a}): {typeof(a)}")
    if isinstance(a, tuple):
        o = a[0]

        # predefined function
        if isinstance(o, str):
            # quantifiers require body boolean
            if o in ("exists", "forall"):
                type_unify("bool", a[2], m)
                return

            # all arguments boolean
            if o in ("and", "or", "eqv", "not"):
                for i in range(1, len(a)):
                    type_unify("bool", a[i], m)
                return

            # all arguments of the same type
            actual = typeof(a[1])
            for i in range(2, len(a)):
                type_unify(actual, a[i], m)
            return

        # user-defined function
        # we already unified the return type
        # by virtue of unifying the type of the whole expression
        # so now just need to unify parameters with arguments
        t = typeof(o)
        assert isinstance(t, tuple)
        for i in range(1, len(a)):
            type_unify(t[i], a[i], m)
        return


# second step of type inference:
# fill in actual types for all the type variables
def type_set(a, m):
    if isinstance(a, tuple):
        for b in a:
            type_set(b, m)
        return
    if isinstance(a, Fn) or isinstance(a, Var):
        a.t = subst(a.t, m)
        if isinstance(a.t, tuple):
            r = []
            for b in a.t:
                if isinstance(b, Var):
                    b = "individual"
                r.append(b)
            a.t = tuple(r)
            return
        if isinstance(a.t, Var):
            a.t = "individual"
            return
        return


# third step of type inference:
# check the types are correct
def type_check(wanted, a):
    if wanted != typeof(a):
        raise ValueError(f"{wanted} != typeof({a})")
    if isinstance(a, tuple):
        o = a[0]

        # predefined function
        if isinstance(o, str):
            # quantifiers require body boolean
            if o in ("exists", "forall"):
                for x in a[1]:
                    if x.t == "bool":
                        raise ValueError(a)
                type_check("bool", a[2])
                return

            # all arguments boolean
            if o in ("and", "or", "eqv", "not"):
                for i in range(1, len(a)):
                    type_check("bool", a[i])
                return

            # all arguments int
            if o in ("div-e", "div-f", "div-t", "rem-e", "rem-f", "rem-t"):
                for i in range(1, len(a)):
                    type_check("int", a[i])

                return

            # all arguments of the same type
            actual = typeof(a[1])
            for i in range(2, len(a)):
                type_check(actual, a[i])

            # =
            if o == "=":
                return

            # numbers
            if actual not in ("int", "rat", "real"):
                raise ValueError(a)

            # rational or real
            if o == "div" and actual == "int":
                raise ValueError(a)
            return

        return
    if isinstance(a, Fn):
        if isinstance(a.t, tuple):
            for b in a.t[1:]:
                if b == "bool":
                    raise ValueError(a)
        return
    if isinstance(a, Var):
        if a.t == "bool":
            raise ValueError(a)
        return


######################################## logic


formula_name_i = 0


def reset_formula_names():
    global formula_name_i
    formula_name_i = 0


class Formula:
    def __init__(self, name, term, inference=None, *parents):
        self.set_name(name)
        self.__term = term
        self.inference = inference
        self.parents = parents

    def proof(self):
        visited = set()
        r = []

        def rec(F):
            if F in visited:
                return
            visited.add(F)
            for G in F.parents:
                rec(G)
            r.append(F)

        rec(self)
        return r

    def set_name(self, name):
        global formula_name_i
        if name is None:
            name = formula_name_i
            formula_name_i += 1
        elif isinstance(name, int):
            formula_name_i = max(formula_name_i, name + 1)
        self.name = name

    def status(self):
        # input data or definition is logical data
        if not self.parents:
            return "lda"

        # negated conjecture is counterequivalent
        if self.inference == "negate":
            return "ceq"

        # if a formula introduces new symbols, then it is only equisatisfiable
        # this happens during subformula renaming in CNF conversion
        if len(self.parents) == 1 and not fns(self.term()).issubset(
            fns(self.parents[0].term())
        ):
            return "esa"

        # formula is a theorem of parents
        # could also be equivalent; don't currently bother distinguishing that case
        return "thm"

    def term(self):
        return self.__term


class Clause(Formula):
    def __init__(self, name, neg, pos, inference=None, *parents):
        for a in neg:
            check_tuples(a)
        for a in pos:
            check_tuples(a)
        self.set_name(name)
        self.neg = tuple(neg)
        self.pos = tuple(pos)
        self.inference = inference
        self.parents = parents

    def __lt__(self, other):
        return self.size() < other.size()

    def rename_vars(self):
        m = {}
        neg = [rename_vars(a, m) for a in self.neg]
        pos = [rename_vars(a, m) for a in self.pos]
        return Clause("*RENAMED*", neg, pos, "rename_vars", self)

    def simplify(self):
        # simplify terms
        neg = map(simplify, self.neg)
        pos = map(simplify, self.pos)

        # eliminate redundancy
        neg = filter(lambda a: a != True, neg)
        pos = filter(lambda a: a != False, pos)

        # reify iterators
        neg = tuple(neg)
        pos = tuple(pos)

        # check for tautology
        if False in neg or True in pos:
            neg, pos = (), (True,)
        else:
            for a in neg:
                if a in pos:
                    neg, pos = (), (True,)

        # did anything change?
        if (neg, pos) == (self.neg, self.pos):
            return self

        # derived clause
        return Clause(None, neg, pos, "simplify", self)

    def size(self):
        if bag(self) in important:
            return 1
        return term_size(self.neg + self.pos)

    def term(self):
        r = tuple([("not", a) for a in self.neg]) + self.pos
        if not r:
            return False
        if len(r) == 1:
            return r[0]
        return ("or",) + r


class Problem:
    def __init__(self):
        self.formulas = []
        self.clauses = []


def valdict(d):
    return tuple(sorted(d.items()))


def valterm(a):
    if isinstance(a, Var):
        return "X"
    if isinstance(a, tuple):
        r = ["("]
        for i in range(len(a)):
            if i:
                r.append(" ")
            r.append(valterm(a[i]))
        r.append(")")
        return "".join(r)
    assert not isinstance(a, list)
    return str(a)


def bag(C):
    neg = {}
    for a in C.neg:
        inc(neg, valterm(a))
    pos = {}
    for a in C.pos:
        inc(pos, valterm(a))
    return valdict(neg), valdict(pos)


important = set()

######################################## Dimacs


def read_dimacs(filename):
    global header
    neg = []
    pos = []
    for s in open(filename):
        if s[0] in ("c", "\n"):
            if header:
                prn(s[:-1])
            if not hasattr(problem, "expected"):
                if "UNSAT" in s:
                    problem.expected = "Unsatisfiable"
                elif "SAT" in s:
                    problem.expected = "Satisfiable"
            continue
        header = False
        if s[0] == "p":
            continue
        for word in s.split():
            atoms = pos
            if word[0] == "-":
                atoms = neg
                word = word[1:]

            if word == "0":
                problem.clauses.append(Clause(None, neg, pos))
                neg = []
                pos = []
                continue

            int(word)
            a = fn(word)
            a.t = "bool"
            atoms.append(a)
    if neg or pos:
        problem.clauses.append(Clause(None, neg, pos))


######################################## TPTP


defined_types = {
    "$o": "bool",
    "$i": "individual",
    "$int": "int",
    "$rat": "rat",
    "$real": "real",
}

defined_fns = {
    "$ceiling": "ceil",
    "$difference": "-",
    "$floor": "floor",
    "$is_int": "int?",
    "$is_rat": "rat?",
    "$less": "<",
    "$lesseq": "<=",
    "$product": "*",
    "$quotient": "/",
    "$quotient_e": "div-e",
    "$quotient_f": "div-f",
    "$quotient_t": "div-t",
    "$remainder_e": "rem-e",
    "$remainder_f": "rem-f",
    "$remainder_t": "rem-t",
    "$round": "round",
    "$sum": "+",
    "$to_int": "to-int",
    "$to_rat": "to-rat",
    "$to_real": "to-real",
    "$truncate": "trunc",
    "$uminus": "unary-",
}


# parser
class Inappropriate(Exception):
    def __init__(self):
        super().__init__("Inappropriate")


def read_tptp(filename, select=True):
    global header
    fname = os.path.basename(filename)
    text = open(filename).read()
    if text and text[-1] != "\n":
        text += "\n"

    # tokenizer
    ti = 0
    tok = ""

    def err(msg):
        line = 1
        for i in range(ti):
            if text[i] == "\n":
                line += 1
        raise ValueError(f"{filename}:{line}: {repr(tok)}: {msg}")

    def lex():
        nonlocal ti
        nonlocal tok
        while ti < len(text):
            c = text[ti]

            # space
            if c.isspace():
                ti += 1
                continue

            # line comment
            if c in ("%", "#"):
                i = ti
                while text[ti] != "\n":
                    ti += 1
                if not hasattr(problem, "expected"):
                    m = re.match(r"%\s*Status\s*:\s*(\w+)", text[i:ti])
                    if m:
                        problem.expected = m[1]
                continue

            # block comment
            if text[ti : ti + 2] == "/*":
                ti += 2
                while text[ti : ti + 2] != "*/":
                    ti += 1
                ti += 2
                continue

            # word
            if c.isalpha() or c == "$":
                i = ti
                ti += 1
                while text[ti].isalnum() or text[ti] == "_":
                    ti += 1
                tok = text[i:ti]
                return

            # quote
            if c in ("'", '"'):
                i = ti
                ti += 1
                while text[ti] != c:
                    if text[ti] == "\\":
                        ti += 1
                    ti += 1
                ti += 1
                tok = text[i:ti]
                return

            # number
            if c.isdigit() or (c == "-" and text[ti + 1].isdigit()):
                # integer part
                i = ti
                ti += 1
                while text[ti].isalnum():
                    ti += 1

                # rational
                if text[ti] == "/":
                    ti += 1
                    while text[ti].isdigit():
                        ti += 1

                # real
                else:
                    if text[ti] == ".":
                        ti += 1
                        while text[ti].isalnum():
                            ti += 1
                    if text[ti - 1] in ("e", "E") and text[ti] in ("+", "-"):
                        ti += 1
                        while text[ti].isdigit():
                            ti += 1

                tok = text[i:ti]
                return

            # punctuation
            if text[ti : ti + 3] in ("<=>", "<~>"):
                tok = text[ti : ti + 3]
                ti += 3
                return
            if text[ti : ti + 2] in ("!=", "=>", "<=", "~&", "~|"):
                tok = text[ti : ti + 2]
                ti += 2
                return
            tok = c
            ti += 1
            return

        # end of file
        tok = None

    def eat(o):
        if tok == o:
            lex()
            return True

    def expect(o):
        if tok != o:
            err(f"expected '{o}'")
        lex()

    # terms
    def read_name():
        o = tok

        # word
        if o[0].islower():
            lex()
            return o

        # single quoted, equivalent to word
        if o[0] == "'":
            lex()
            return o[1:-1]

        # number
        if o[0].isdigit() or o[0] == "-":
            lex()
            return int(o)

        err("expected name")

    def atomic_type():
        o = tok
        if o in defined_types:
            lex()
            return defined_types[o]
        if tok == "$tType":
            raise Inappropriate()
        return mktype(read_name())

    def compound_type():
        if eat("("):
            params = [atomic_type()]
            while eat("*"):
                params.append(atomic_type())
            expect(")")
            expect(">")
            return (atomic_type(),) + tuple(params)
        t = atomic_type()
        if eat(">"):
            return atomic_type(), t
        return t

    free = {}

    def args(bound, n=-1):
        expect("(")
        r = []
        if tok != ")":
            r.append(atomic_term(bound))
            while tok == ",":
                lex()
                r.append(atomic_term(bound))
        if n > 0 and len(r) != n:
            err(f"expected {n} args")
        expect(")")
        return tuple(r)

    def atomic_term(bound):
        o = tok

        # defined function
        if o[0] == "$":
            # constant
            if eat("$false"):
                return False
            if eat("$true"):
                return True

            # syntax sugar
            if eat("$greater"):
                s = args(bound, 2)
                return "<", s[1], s[0]
            if eat("$greatereq"):
                s = args(bound, 2)
                return "<=", s[1], s[0]
            if eat("$distinct"):
                s = args()
                inequalities = ["and"]
                for i in range(len(s)):
                    for j in range(len(s)):
                        if i != j:
                            inequalities.append(("not", ("=", s[i], s[j])))
                return tuple(inequalities)

            # predefined function
            if o in defined_fns:
                a = defined_fns[o]
                lex()
                arities = {
                    "*": 2,
                    "+": 2,
                    "-": 2,
                    "/": 2,
                    "<": 2,
                    "<=": 2,
                    "ceil": 1,
                    "div-e": 2,
                    "div-f": 2,
                    "div-t": 2,
                    "floor": 1,
                    "int?": 1,
                    "rat?": 1,
                    "rem-e": 2,
                    "rem-f": 2,
                    "rem-t": 2,
                    "round": 1,
                    "to-int": 1,
                    "to-rat": 1,
                    "to-real": 1,
                    "trunc": 1,
                    "unary-": 1,
                }
                return (a,) + args(bound, arities[a])
            err("unknown function")

        # distinct object
        if o[0] == '"':
            lex()
            return distinct_object(o)

        # number
        if o[0].isdigit() or o[0] == "-":
            lex()
            try:
                return int(o)
            except ValueError:
                if "/" in o:
                    return fractions.Fraction(o)
                return Real(o)

        # variable
        if o[0].isupper():
            lex()
            if o in bound:
                return bound[o]
            if o in free:
                return free[o]
            a = Var("individual")
            free[o] = a
            return a

        # higher-order terms
        if tok == "!":
            raise Inappropriate()

        # function
        a = fn(read_name())
        if tok == "(":
            s = args(bound)
            if not hasattr(a, "t"):
                a.type_args(Var(), s)
            return (a,) + s
        if not hasattr(a, "t"):
            a.t = Var()
        return a

    def infix_unary(bound):
        a = atomic_term(bound)
        o = tok
        if o == "=":
            lex()
            return "=", a, atomic_term(bound)
        if o == "!=":
            lex()
            return "not", ("=", a, atomic_term(bound))
        return a

    def var(bound):
        o = tok
        if not o[0].isupper():
            err("expected variable")
        lex()
        t = "individual"
        if eat(":"):
            t = atomic_type()
        a = Var(t)
        bound[o] = a
        return a

    def unitary_formula(bound):
        o = tok
        if o == "(":
            lex()
            a = logic_formula(bound)
            expect(")")
            return a
        if o == "~":
            lex()
            return "not", unitary_formula(bound)
        if o in ("!", "?"):
            o = "exists" if o == "?" else "forall"
            lex()

            # variables
            bound = bound.copy()
            expect("[")
            v = []
            v.append(var(bound))
            while tok == ",":
                lex()
                v.append(var(bound))
            expect("]")

            # body
            expect(":")
            a = o, tuple(v), unitary_formula(bound)
            return a
        return infix_unary(bound)

    def logic_formula(bound):
        a = unitary_formula(bound)
        o = tok
        if o == "&":
            r = ["and", a]
            while eat("&"):
                r.append(unitary_formula(bound))
            return tuple(r)
        if o == "|":
            r = ["or", a]
            while eat("|"):
                r.append(unitary_formula(bound))
            return tuple(r)
        if o == "=>":
            lex()
            return imp(a, unitary_formula(bound))
        if o == "<=":
            lex()
            return imp(unitary_formula(bound), a)
        if o == "<=>":
            lex()
            return "eqv", a, unitary_formula(bound)
        if o == "<~>":
            lex()
            return "not", ("eqv", a, unitary_formula(bound))
        if o == "~&":
            lex()
            return "not", ("and", a, unitary_formula(bound))
        if o == "~|":
            lex()
            return "not", ("or", a, unitary_formula(bound))
        return a

    # top level
    def ignore():
        if eat("("):
            while not eat(")"):
                ignore()
            return
        lex()

    def selecting(name):
        return select is True or name in select

    def annotated_clause():
        lex()
        expect("(")

        # name
        name = read_name()
        expect(",")

        # role
        role = read_name()
        expect(",")

        # formula
        parens = eat("(")
        neg = []
        pos = []
        while True:
            a = unitary_formula({})
            if isinstance(a, tuple) and a[0] == "not":
                neg.append(a[1])
            else:
                pos.append(a)
            if tok != "|":
                break
            lex()
        if selecting(name):
            C = Clause(name, neg, pos)
            C.fname = fname
            C.role = role
            problem.clauses.append(C)
        if parens:
            expect(")")

        # annotations
        if tok == ",":
            while tok != ")":
                ignore()

        # end
        expect(")")
        expect(".")

    def include():
        lex()
        expect("(")

        # tptp
        tptp = os.getenv("TPTP")
        if not tptp:
            err("TPTP environment variable not set")

        # file
        filename1 = read_name()

        # select
        select1 = select
        if eat(","):
            expect("[")
            select1 = []
            while True:
                name = read_name()
                if selecting(name):
                    select1.append(name)
                if not eat(","):
                    break
            expect("]")

        # include
        read_tptp(tptp + "/" + filename1, select1)

        # end
        expect(")")
        expect(".")

    lex()
    header = False
    while tok:
        free.clear()
        if tok == "cnf":
            annotated_clause()
            continue
        if tok in ("fof", "tff"):
            annotated_formula()
            continue
        if tok == "include":
            include()
            continue
        err("unknown language")


def read_proof(text):
    if text and text[-1] != "\n":
        text += "\n"

    # tokenizer
    ti = 0
    tok = ""

    def err(msg):
        line = 1
        for i in range(ti):
            if text[i] == "\n":
                line += 1
        raise ValueError(f"{filename}:{line}: {repr(tok)}: {msg}")

    def lex():
        nonlocal ti
        nonlocal tok
        while ti < len(text):
            c = text[ti]

            # space
            if c.isspace():
                ti += 1
                continue

            # line comment
            if c in ("%", "#"):
                i = ti
                while text[ti] != "\n":
                    ti += 1
                if not hasattr(problem, "expected"):
                    m = re.match(r"%\s*Status\s*:\s*(\w+)", text[i:ti])
                    if m:
                        problem.expected = m[1]
                continue

            # block comment
            if text[ti : ti + 2] == "/*":
                ti += 2
                while text[ti : ti + 2] != "*/":
                    ti += 1
                ti += 2
                continue

            # word
            if c.isalpha() or c == "$":
                i = ti
                ti += 1
                while text[ti].isalnum() or text[ti] == "_":
                    ti += 1
                tok = text[i:ti]
                return

            # quote
            if c in ("'", '"'):
                i = ti
                ti += 1
                while text[ti] != c:
                    if text[ti] == "\\":
                        ti += 1
                    ti += 1
                ti += 1
                tok = text[i:ti]
                return

            # number
            if c.isdigit() or (c == "-" and text[ti + 1].isdigit()):
                # integer part
                i = ti
                ti += 1
                while text[ti].isalnum():
                    ti += 1

                # rational
                if text[ti] == "/":
                    ti += 1
                    while text[ti].isdigit():
                        ti += 1

                # real
                else:
                    if text[ti] == ".":
                        ti += 1
                        while text[ti].isalnum():
                            ti += 1
                    if text[ti - 1] in ("e", "E") and text[ti] in ("+", "-"):
                        ti += 1
                        while text[ti].isdigit():
                            ti += 1

                tok = text[i:ti]
                return

            # punctuation
            if text[ti : ti + 3] in ("<=>", "<~>"):
                tok = text[ti : ti + 3]
                ti += 3
                return
            if text[ti : ti + 2] in ("!=", "=>", "<=", "~&", "~|"):
                tok = text[ti : ti + 2]
                ti += 2
                return
            tok = c
            ti += 1
            return

        # end of file
        tok = None

    def eat(o):
        if tok == o:
            lex()
            return True

    def expect(o):
        if tok != o:
            err(f"expected '{o}'")
        lex()

    # terms
    def read_name():
        o = tok

        # word
        if o[0].islower():
            lex()
            return o

        # single quoted, equivalent to word
        if o[0] == "'":
            lex()
            return o[1:-1]

        # number
        if o[0].isdigit() or o[0] == "-":
            lex()
            return int(o)

        err("expected name")

    def atomic_type():
        o = tok
        if o in defined_types:
            lex()
            return defined_types[o]
        if tok == "$tType":
            raise Inappropriate()
        return mktype(read_name())

    def compound_type():
        if eat("("):
            params = [atomic_type()]
            while eat("*"):
                params.append(atomic_type())
            expect(")")
            expect(">")
            return (atomic_type(),) + tuple(params)
        t = atomic_type()
        if eat(">"):
            return atomic_type(), t
        return t

    free = {}

    def args(bound, n=-1):
        expect("(")
        r = []
        if tok != ")":
            r.append(atomic_term(bound))
            while tok == ",":
                lex()
                r.append(atomic_term(bound))
        if n > 0 and len(r) != n:
            err(f"expected {n} args")
        expect(")")
        return tuple(r)

    def atomic_term(bound):
        o = tok

        # defined function
        if o[0] == "$":
            # constant
            if eat("$false"):
                return False
            if eat("$true"):
                return True

            # syntax sugar
            if eat("$greater"):
                s = args(bound, 2)
                return "<", s[1], s[0]
            if eat("$greatereq"):
                s = args(bound, 2)
                return "<=", s[1], s[0]
            if eat("$distinct"):
                s = args()
                inequalities = ["and"]
                for i in range(len(s)):
                    for j in range(len(s)):
                        if i != j:
                            inequalities.append(("not", ("=", s[i], s[j])))
                return tuple(inequalities)

            # predefined function
            if o in defined_fns:
                a = defined_fns[o]
                lex()
                arities = {
                    "*": 2,
                    "+": 2,
                    "-": 2,
                    "/": 2,
                    "<": 2,
                    "<=": 2,
                    "ceil": 1,
                    "div-e": 2,
                    "div-f": 2,
                    "div-t": 2,
                    "floor": 1,
                    "int?": 1,
                    "rat?": 1,
                    "rem-e": 2,
                    "rem-f": 2,
                    "rem-t": 2,
                    "round": 1,
                    "to-int": 1,
                    "to-rat": 1,
                    "to-real": 1,
                    "trunc": 1,
                    "unary-": 1,
                }
                return (a,) + args(bound, arities[a])
            err("unknown function")

        # distinct object
        if o[0] == '"':
            lex()
            return distinct_object(o)

        # number
        if o[0].isdigit() or o[0] == "-":
            lex()
            try:
                return int(o)
            except ValueError:
                if "/" in o:
                    return fractions.Fraction(o)
                return Real(o)

        # variable
        if o[0].isupper():
            lex()
            if o in bound:
                return bound[o]
            if o in free:
                return free[o]
            a = Var("individual")
            free[o] = a
            return a

        # higher-order terms
        if tok == "!":
            raise Inappropriate()

        # function
        a = fn(read_name())
        if tok == "(":
            s = args(bound)
            if not hasattr(a, "t"):
                a.type_args(Var(), s)
            return (a,) + s
        if not hasattr(a, "t"):
            a.t = Var()
        return a

    def infix_unary(bound):
        a = atomic_term(bound)
        o = tok
        if o == "=":
            lex()
            return "=", a, atomic_term(bound)
        if o == "!=":
            lex()
            return "not", ("=", a, atomic_term(bound))
        return a

    def var(bound):
        o = tok
        if not o[0].isupper():
            err("expected variable")
        lex()
        t = "individual"
        if eat(":"):
            t = atomic_type()
        a = Var(t)
        bound[o] = a
        return a

    def unitary_formula(bound):
        o = tok
        if o == "(":
            lex()
            a = logic_formula(bound)
            expect(")")
            return a
        if o == "~":
            lex()
            return "not", unitary_formula(bound)
        if o in ("!", "?"):
            o = "exists" if o == "?" else "forall"
            lex()

            # variables
            bound = bound.copy()
            expect("[")
            v = []
            v.append(var(bound))
            while tok == ",":
                lex()
                v.append(var(bound))
            expect("]")

            # body
            expect(":")
            a = o, tuple(v), unitary_formula(bound)
            return a
        return infix_unary(bound)

    def logic_formula(bound):
        a = unitary_formula(bound)
        o = tok
        if o == "&":
            r = ["and", a]
            while eat("&"):
                r.append(unitary_formula(bound))
            return tuple(r)
        if o == "|":
            r = ["or", a]
            while eat("|"):
                r.append(unitary_formula(bound))
            return tuple(r)
        if o == "=>":
            lex()
            return imp(a, unitary_formula(bound))
        if o == "<=":
            lex()
            return imp(unitary_formula(bound), a)
        if o == "<=>":
            lex()
            return "eqv", a, unitary_formula(bound)
        if o == "<~>":
            lex()
            return "not", ("eqv", a, unitary_formula(bound))
        if o == "~&":
            lex()
            return "not", ("and", a, unitary_formula(bound))
        if o == "~|":
            lex()
            return "not", ("or", a, unitary_formula(bound))
        return a

    # top level
    def ignore():
        if eat("("):
            while not eat(")"):
                ignore()
            return
        lex()

    def selecting(name):
        return select is True or name in select

    def annotated_clause():
        lex()
        expect("(")

        # name
        name = read_name()
        expect(",")

        # role
        role = read_name()
        expect(",")

        # formula
        parens = eat("(")
        neg = []
        pos = []
        while True:
            a = unitary_formula({})
            if isinstance(a, tuple) and a[0] == "not":
                neg.append(a[1])
            else:
                pos.append(a)
            if tok != "|":
                break
            lex()
        C = Clause(name, neg, pos)
        C.fname = ""
        C.role = role
        problem.clauses.append(C)
        if parens:
            expect(")")

        # annotations
        if tok == ",":
            while tok != ")":
                ignore()

        # end
        expect(")")
        expect(".")

    lex()
    header = False
    while tok:
        free.clear()
        if tok == "cnf":
            annotated_clause()
            continue
        if tok in ("fof", "tff"):
            annotated_formula()
            continue
        if tok == "include":
            include()
            continue
        err("unknown language")


# print
var_name_i = 0


def reset_var_names():
    global var_name_i
    var_name_i = 0


def set_var_name(x):
    global var_name_i
    if hasattr(x, "name"):
        return
    i = var_name_i
    var_name_i += 1
    if i < 26:
        x.name = chr(65 + i)
    else:
        x.name = "Z" + str(i - 25)


def prargs(a):
    pr("(")
    for i in range(1, len(a)):
        if i > 1:
            pr(",")
        prterm(a[i])
    pr(")")


def need_parens(a, parent):
    if not parent:
        return
    if a[0] in ("and", "eqv", "or"):
        return parent[0] in ("and", "eqv", "exists", "forall", "not", "or")


def prterm(a, parent=None):
    if isinstance(a, tuple):
        o = a[0]
        # infix
        if o == "=":
            prterm(a[1])
            pr("=")
            prterm(a[2])
            return
        connectives = {"and": "&", "eqv": "<=>", "or": "|"}
        if o in connectives:
            if need_parens(a, parent):
                pr("(")
            assert len(a) >= 3
            for i in range(1, len(a)):
                if i > 1:
                    pr(f" {connectives[o]} ")
                prterm(a[i], a)
            if need_parens(a, parent):
                pr(")")
            return

        # prefix/infix
        if o == "not":
            if isinstance(a[1], tuple) and a[1][0] == "=":
                a = a[1]
                prterm(a[1])
                pr("!=")
                prterm(a[2])
                return
            pr("~")
            prterm(a[1], a)
            return

        # prefix
        if o in ("exists", "forall"):
            if o == "exists":
                pr("?")
            else:
                pr("!")
            pr("[")
            v = a[1]
            for i in range(len(v)):
                if i:
                    pr(",")
                x = v[i]
                set_var_name(x)
                pr(x)
                if x.t != "individual":
                    pr(":")
                    prtype(x.t)
            pr("]:")
            prterm(a[2], a)
            return
        if isinstance(o, str):
            pr(invert(defined_fns)[o])
        else:
            pr(o)
        prargs(a)
        return
    if a is False:
        pr("$false")
        return
    if a is True:
        pr("$true")
        return
    if isinstance(a, Var):
        set_var_name(a)
    pr(a)


def prtype(a):
    if isinstance(a, str):
        pr(invert(defined_types)[a])
        return
    pr(a)


def prformula(F):
    reset_var_names()
    if isinstance(F, Clause):
        pr("cnf")
    else:
        pr("fof")
    pr("(")

    # name
    pr(F.name)
    pr(", ")

    # role
    if hasattr(F, "role"):
        pr(F.role)
    else:
        pr("plain")
    pr(", ")

    # content
    a = F.term()
    if not isinstance(F, Clause):
        a = quantify(a)
    prterm(a)
    pr(", ")

    # source
    if hasattr(F, "fname"):
        pr(f"file('{F.fname}',{F.name})")
    elif F.inference:
        pr(f"inference({F.inference},[status({F.status()})],[")
        for i in range(len(F.parents)):
            if i:
                pr(",")
            pr(F.parents[i].name)
        pr("])")
    else:
        pr("introduced(definition)")

    # end
    prn(").")


######################################## read and prepare


def read_problem(filename):
    global header
    global problem

    # init
    clear_fns()
    header = True
    problem = Problem()
    reset_formula_names()

    # read
    if os.path.splitext(filename)[1] == ".cnf":
        read_dimacs(filename)
    else:
        read_tptp(filename)

    # infer types
    terms = [F.term() for F in problem.formulas + problem.clauses]
    m = {}
    for a in terms:
        type_unify("bool", a, m)
    for a in terms:
        type_set(a, m)
    for a in terms:
        type_check("bool", a)

    return problem


######################################## subsumption

# something of an open question:
# https://stackoverflow.com/questions/54043747/clause-subsumption-algorithm
def subsumes(C, D):
    # negative and positive literals must subsume separately
    c1 = C.neg
    c2 = C.pos
    d1 = D.neg
    d2 = D.pos

    # longer clause cannot subsume shorter one
    if len(c1) > len(d1) or len(c2) > len(d2):
        return

    # fewer literals typically fail faster, so try fewer side first
    if len(c2) + len(d2) < len(c1) + len(d1):
        c1, c2 = c2, c1
        d1, d2 = d2, d1

    # search with timeout
    steps = 0

    def search(c1, c2, d1, d2, m):
        nonlocal steps

        # worst-case time is exponential
        # so give up if taking too long
        if steps == 1000:
            raise Timeout()
        steps += 1

        # matched everything in one polarity?
        if not c1:
            # matched everything in the other polarity?
            if not c2:
                return m

            # try the other polarity
            return search(c2, None, d2, None, m)

        # try matching literals
        for ci in range(len(c1)):
            ce = equation(c1[ci])
            for di in range(len(d1)):
                de = equation(d1[di])

                # try orienting equation one way
                m1 = m.copy()
                if match(ce[1], de[1], m1) and match(ce[2], de[2], m1):
                    m1 = search(remove(c1, ci), c2, remove(d1, di), d2, m1)
                    if m1 is not None:
                        return m1

                # and the other way
                m1 = m.copy()
                if match(ce[1], de[2], m1) and match(ce[2], de[1], m1):
                    m1 = search(remove(c1, ci), c2, remove(d1, di), d2, m1)
                    if m1 is not None:
                        return m1

    try:
        m = search(c1, c2, d1, d2, {})
        return m is not None
    except Timeout:
        pass


def forward_subsumes(Ds, C):
    for D in Ds:
        if hasattr(D, "dead"):
            continue
        if subsumes(D, C):
            return True


def backward_subsume(C, Ds):
    for D in Ds:
        if hasattr(D, "dead"):
            continue
        if subsumes(C, D):
            D.dead = True


######################################## superposition


# partial implementation of the superposition calculus
# a full implementation would also implement an order on equations
# e.g. lexicographic path ordering or Knuth-Bendix ordering
def original(C):
    if C.inference == "rename_vars":
        return C.parents[0]
    return C


def clause(m, neg, pos, inference, *parents):
    # check_limits()
    neg = subst(tuple(neg), m)
    pos = subst(tuple(pos), m)
    C = Clause(None, neg, pos, inference, *map(original, parents)).simplify()
    if C.term() is True:
        return
    if C.size() > 10_000_000:
        raise ResourceOut()
    clauses.append(C)


# equality resolution
# C | c0 != c1
# ->
# C/m
# where
# m = unify(c0, c1)

# for each negative equation
def resolution(C):
    for ci in range(len(C.neg)):
        _, c0, c1 = equation(C.neg[ci])
        m = {}
        if unify(c0, c1, m):
            resolutionc(C, ci, m)


# substitute and make new clause
def resolutionc(C, ci, m):
    neg = remove(C.neg, ci)
    pos = C.pos
    clause(m, neg, pos, "resolve", C)


# equality factoring
# C | c0 = c1 | c2 = c3
# ->
# (C | c0 = c1 | c1 != c3)/m
# where
# m = unify(c0, c2)

# for each positive equation (both directions)
def factoring(C):
    for ci in range(len(C.pos)):
        _, c0, c1 = equation(C.pos[ci])
        factoring1(C, ci, c0, c1)
        factoring1(C, ci, c1, c0)


# for each positive equation (both directions) again
def factoring1(C, ci, c0, c1):
    for cj in range(len(C.pos)):
        if cj == ci:
            continue
        _, c2, c3 = equation(C.pos[cj])
        factoringc(C, c0, c1, cj, c2, c3)
        factoringc(C, c0, c1, cj, c3, c2)


# check, substitute and make new clause
def factoringc(C, c0, c1, cj, c2, c3):
    if not equatable(c1, c3):
        return
    m = {}
    if not unify(c0, c2, m):
        return
    neg = C.neg + (equation_atom(c1, c3),)
    pos = remove(C.pos, cj)
    clause(m, neg, pos, "factor", C)


# negative superposition
# C | c0 = c1, D | d0(a) != d1
# ->
# (C | D | d0(c1) != d1)/m
# where
# m = unify(c0, a)
# a not variable

# for each positive equation in C (both directions)
def superposition_neg(C, D):
    for ci in range(len(C.pos)):
        _, c0, c1 = equation(C.pos[ci])
        superposition_neg1(C, D, ci, c0, c1)
        superposition_neg1(C, D, ci, c1, c0)


# for each negative equation in D (both directions)
def superposition_neg1(C, D, ci, c0, c1):
    if c0 is True:
        return
    for di in range(len(D.neg)):
        _, d0, d1 = equation(D.neg[di])
        superposition_neg2(C, D, ci, c0, c1, di, d0, d1, [], d0)
        superposition_neg2(C, D, ci, c0, c1, di, d1, d0, [], d1)


# descend into subterms
def superposition_neg2(C, D, ci, c0, c1, di, d0, d1, path, a):
    if isinstance(a, Var):
        return
    superposition_negc(C, D, ci, c0, c1, di, d0, d1, path, a)
    if isinstance(a, tuple):
        for i in range(1, len(a)):
            path.append(i)
            superposition_neg2(C, D, ci, c0, c1, di, d0, d1, path, a[i])
            path.pop()


# check, substitute and make new clause
def superposition_negc(C, D, ci, c0, c1, di, d0, d1, path, a):
    m = {}
    if not unify(c0, a, m):
        return
    neg = C.neg + remove(D.neg, di) + (equation_atom(splice(d0, path, c1), d1),)
    pos = remove(C.pos, ci) + D.pos
    clause(m, neg, pos, "ns", original(C), original(D))


# positive superposition
# C | c0 = c1, D | d0(a) = d1
# ->
# (C | D | d0(c1) = d1)/m
# where
# m = unify(c0, a)
# a not variable

# for each positive equation in C (both directions)
def superposition_pos(C, D):
    for ci in range(len(C.pos)):
        _, c0, c1 = equation(C.pos[ci])
        superposition_pos1(C, D, ci, c0, c1)
        superposition_pos1(C, D, ci, c1, c0)


# for each positive equation in D (both directions)
def superposition_pos1(C, D, ci, c0, c1):
    if c0 is True:
        return
    for di in range(len(D.pos)):
        _, d0, d1 = equation(D.pos[di])
        superposition_pos2(C, D, ci, c0, c1, di, d0, d1, [], d0)
        superposition_pos2(C, D, ci, c0, c1, di, d1, d0, [], d1)


# descend into subterms
def superposition_pos2(C, D, ci, c0, c1, di, d0, d1, path, a):
    if isinstance(a, Var):
        return
    superposition_posc(C, D, ci, c0, c1, di, d0, d1, path, a)
    if isinstance(a, tuple):
        for i in range(1, len(a)):
            path.append(i)
            superposition_pos2(C, D, ci, c0, c1, di, d0, d1, path, a[i])
            path.pop()


# check, substitute and make new clause
def superposition_posc(C, D, ci, c0, c1, di, d0, d1, path, a):
    m = {}
    if not unify(c0, a, m):
        return
    neg = C.neg + D.neg
    pos = (
        remove(C.pos, ci)
        + remove(D.pos, di)
        + (equation_atom(splice(d0, path, c1), d1),)
    )
    clause(m, neg, pos, "ps", original(C), original(D))


# superposition is incomplete on arithmetic
def contains_arithmetic(a):
    if isinstance(a, tuple):
        for b in a[1:]:
            if contains_arithmetic(b):
                return True
    return typeof(a) in ("int", "rat", "real")


def solve(Cs):
    global clauses
    unprocessed = [C.simplify() for C in Cs]
    heapq.heapify(unprocessed)
    processed = []

    # Otter loop
    # in tests performs about as well as Discount loop
    # and uses less memory
    prev = 999999
    prevp = 999999
    while unprocessed:
        # given clause
        g = heapq.heappop(unprocessed)
        if important:
            im = set(important)
            for p in processed:
                im.discard(bag(p))
            if len(im) != prev or (
                len(processed) != prevp and len(processed) % 100 == 0
            ):
                prev = len(im)
                prevp = len(processed)
                print(
                    "%d\t%d\t%d\t%d"
                    % (len(processed), len(unprocessed), len(important), len(im))
                )
                print(im)
            # print(bag(g)in important)

        # subsumption
        if hasattr(g, "dead"):
            continue

        # solved?
        if g.term() is False:
            return "Unsatisfiable", g

        # match/unify assume clauses have disjoint variable names
        C = g.rename_vars()

        # subsumption
        if forward_subsumes(processed, C):
            continue
        if forward_subsumes(unprocessed, C):
            continue
        backward_subsume(C, processed)
        backward_subsume(C, unprocessed)

        # may need to match g with itself
        processed.append(g)

        # generate new clauses
        clauses = []
        resolution(C)
        factoring(C)
        for D in processed:
            if hasattr(D, "dead"):
                continue
            superposition_neg(C, D)
            superposition_neg(D, C)
            superposition_pos(C, D)
            superposition_pos(D, C)
        for C in clauses:
            heapq.heappush(unprocessed, C)

    # first-order logic is not complete on arithmetic
    for C in Cs:
        for a in C.neg + C.pos:
            if contains_arithmetic(a):
                return "GaveUp", None
    return "Satisfiable", None


######################################## top level

results = {}


def solved(r):
    if r == "Unsatisfiable":
        return 1
    if r == "Satisfiable":
        return 1


def inc(d, k):
    if k not in d:
        d[k] = 0
    d[k] += 1


def difficulty(f):
    xs = read_lines(f)
    for x in xs:
        m = re.match(r"% Rating   : (\d+\.\d+)", x)
        if m:
            return m[1]
    return "?"


def run_base(filename):
    start = time.time()
    set_timeout()
    try:
        problem = read_problem(filename)
        r, conclusion = solve(problem.clauses)
        if r in (
            "Unsatisfiable",
            "ContradictoryAxioms",
            "Satisfiable",
        ):
            if hasattr(problem, "expected") and r != problem.expected:
                if problem.expected == "ContradictoryAxioms" and r in (
                    "Theorem",
                    "Unsatisfiable",
                ):
                    pass
                else:
                    raise ValueError(f"{r} != {problem.expected}")
    except (Inappropriate, Timeout) as e:
        r = str(e)
    except RecursionError:
        r = "ResourceOut"
    return r


def run_e(f):
    cmd = ["bin/eprover", "-s", "-p", f]
    try:
        p = subprocess.run(
            cmd,
            capture_output=True,
            encoding="utf-8",
            timeout=10,
            check=True,
        )
        if "Proof found" in p.stdout:
            r = "Unsatisfiable"
        elif "No proof found":
            r = "Satisfiable"
        else:
            raise Exception(p.stdout)
        return r, p.stdout
    except subprocess.TimeoutExpired:
        r = "Timeout"
        return r, ""


def do_file(filename):
    global solved_base
    global unsolved_e
    global solved_guided
    global important

    # list file
    if os.path.splitext(filename)[1] == ".lst":
        for s in open(filename):
            do_file(s.strip())
        return

    fname = os.path.basename(filename)
    important = set()

    read_problem(filename)

    r_e, proof_e = run_e(filename)
    print(r_e)
    assert solved(r_e)

    problem.clauses = []
    read_proof(proof_e)
    for C in problem.clauses:
        important.add(bag(C))
    r_guided = run_base(filename)
    print(r_guided, flush=True)
    if solved(r_guided):
        solved_guided += 1


parser = argparse.ArgumentParser()
parser.add_argument("files", nargs="+")
args = parser.parse_args()
start = time.time()
solved_base = 0
unsolved_e = 0
solved_guided = 0
for filename in args.files:
    if os.path.isfile(filename):
        do_file(filename)
        continue
    for root, dirs, files in os.walk(filename):
        for fname in files:
            do_file(os.path.join(root, fname))
print()
prn(f"{time.time() - start:.3f} seconds")
