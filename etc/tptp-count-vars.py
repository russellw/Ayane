#!/usr/bin/python3
# Scan TPTP problems to gather statistics on max number of variables in a single formula
# answer: 34940
import argparse
import itertools
import os
import re
import sys
import time

# numbers larger than 2000 silently fail
sys.setrecursionlimit(2000)


######################################## terms


skolemPrefix = "sK"


def boolean(b):
    if b:
        return "$true"
    return "$false"


def compound(a):
    return type(a) in (list, tuple)


def imp(a, b):
    return "|", ("~", a), b


def tuplify(a):
    if not compound(a):
        return a
    r = []
    for b in a:
        r.append(tuplify(b))
    return tuple(r)


######################################## parser

nvars = 0
maxnvars = 0


def normal(s):
    if s[0].isupper():
        return
    for c in s:
        if not c.isalnum():
            return
    return 1


def parse(f, select=True):
    global nvars
    global maxnvars
    text = open(f).read()
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
        raise Exception(f"{f}:{line}: {repr(tok)}: {msg}")

    def quote():
        nonlocal ti
        nonlocal tok
        i = ti
        q = text[i]
        ti += 1
        while text[ti] != q:
            if text[ti] == "\\":
                ti += 1
            ti += 1
        ti += 1
        tok = text[i:ti]

    def lex():
        global expected
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
                continue

            # block comment
            if text[ti : ti + 2] == "/*":
                ti += 2
                while text[ti : ti + 2] != "*/":
                    ti += 1
                ti += 2
                continue

            # word
            if c.isalnum() or c == "$":
                i = ti
                ti += 1
                while text[ti].isalnum() or text[ti] == "_":
                    ti += 1
                tok = text[i:ti]
                if tok.startswith(skolemPrefix):
                    err("skolem prefix found")
                return

            # quoted word
            if c == "'":
                quote()
                if normal(tok[1:-1]):
                    tok = tok[1:-1]
                return

            # distinct object
            if c == '"':
                quote()
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
    def word():
        o = tok
        c = o[0]
        if c.islower() or c.isdigit() or c == "'":
            lex()
            return o
        err("expected name")

    def atomicTerm(m):
        o = tok
        c = o[0]

        # variable
        if c.isupper():
            lex()
            return m[o]

        # defined term or distinct object
        if c == "$" or c == '"':
            lex()
            return o

        # function
        a = word()
        if eat("("):
            a = [a]
            while 1:
                a.append(atomicTerm(m))
                if not eat(","):
                    break
            expect(")")
        return a

    def infixUnary(m):
        a = atomicTerm(m)
        o = tok
        if o == "=":
            lex()
            return o, a, atomicTerm(m)
        if o == "!=":
            lex()
            return "~", ("=", a, atomicTerm(m))
        return a

    def unitaryFormula(m):
        global nvars
        o = tok
        if o == "(":
            lex()
            a = logicFormula(m)
            expect(")")
            return a
        if o == "~":
            lex()
            return "~", unitaryFormula(m)
        if o in ("!", "?"):
            lex()

            # variables
            m = m.copy()
            expect("[")
            v = []
            while 1:
                if not tok[0].isupper():
                    err("expected variable")
                x = tok
                m[tok] = x
                v.append(x)
                nvars += 1
                lex()
                if not eat(","):
                    break
            expect("]")

            # body
            expect(":")
            return o, v, unitaryFormula(m)
        return infixUnary(m)

    def logicFormula(m):
        a = unitaryFormula(m)
        o = tok
        if o in ("&", "|"):
            r = [o, a]
            while eat(o):
                r.append(unitaryFormula(m))
            return r
        if o == "=>":
            lex()
            return imp(a, unitaryFormula(m))
        if o == "<=":
            lex()
            return imp(unitaryFormula(m), a)
        if o == "<=>":
            lex()
            return o, a, unitaryFormula(m)
        if o == "<~>":
            lex()
            return "~", ("<=>", a, unitaryFormula(m))
        if o == "~&":
            lex()
            return "~", ("&", a, unitaryFormula(m))
        if o == "~|":
            lex()
            return "~", ("|", a, unitaryFormula(m))
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

    def annotatedFormula():
        expect("(")

        # name
        name = word()
        expect(",")

        # role
        role = word()
        expect(",")

        # formula
        a = logicFormula({})
        if selecting(name):
            if role == "conjecture":
                a = "~", a
            formulas.append(a)

        # annotations
        if tok == ",":
            while tok != ")":
                ignore()

        # end
        expect(")")
        expect(".")

    def include():
        expect("(")

        # tptp
        tptp = os.getenv("TPTP")
        if not tptp:
            err("TPTP environment variable not set")

        # file
        g = word()[1:-1]

        # select
        select1 = select
        if eat(","):
            expect("[")
            select1 = []
            while 1:
                name = word()
                if selecting(name):
                    select1.append(name)
                if not eat(","):
                    break
            expect("]")

        # include
        parse(tptp + "/" + g, select1)

        # end
        expect(")")
        expect(".")

    lex()
    while tok:
        if eat("include"):
            include()
            continue
        expect("fof")
        nvars = 0
        annotatedFormula()
        maxnvars = max(maxnvars, nvars)


######################################## top level

term_set = set()
term_list = []
term_setn = 0
term_listn = 0


def complexFormula(a):
    if not compound(a):
        return
    if a[0] == "<=>":
        return 1
    if a[0] == "~":
        return complexFormula(a[1])
    if a[0] in ("!", "?"):
        return complexFormula(a[2])
    if a[0] in ("&", "|"):
        for b in a[1:]:
            if complexFormula(b):
                return 1


def scan(a):
    if complexFormula(a):
        term_set.add(a)
        term_list.append(a)
        for b in a:
            scan(b)


def eligible(f):
    f = f.strip()
    return os.path.splitext(f)[1] == ".p" and "+" in f


def doFile(f):
    global nvars
    global maxnvars
    global formulas
    global term_set
    global term_setn
    global term_list
    global term_listn
    nvars = 0
    formulas = []
    term_set = set()
    term_list = []

    try:
        parse(f)
    except RecursionError:
        pass


parser = argparse.ArgumentParser()
parser.add_argument("files", nargs="+")
args = parser.parse_args()
for arg in args.files:
    if os.path.isfile(arg):
        if os.path.splitext(arg)[1] == ".lst":
            for f in open(arg):
                if eligible(f):
                    doFile(f.strip())
            continue
        doFile(arg)
        continue
    for root, dirs, files in os.walk(arg):
        for f in files:
            if eligible(f):
                doFile(os.path.join(root, f))
print(maxnvars)
