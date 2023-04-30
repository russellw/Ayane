import argparse
import os
import re
import sys

parser = argparse.ArgumentParser()
parser.add_argument("files", nargs="*")
args = parser.parse_args()

if not args.files:
    args.files = ["tptp"]

tptp = os.getenv("TPTP")
if not tptp:
    raise Exception("TPTP environment variable not set")

problems = []
for arg in args.files:
    if arg.lower() == "tptp":
        arg = tptp
    elif re.match(r"[A-Za-z][A-Za-z][A-Za-z]$", arg):
        arg = arg.upper()
        arg = os.path.join(tptp, "Problems", arg)
    elif re.match(r"[A-Za-z][A-Za-z][A-Za-z]\d\d\d.\d+$", arg):
        arg = arg.upper()
        arg = os.path.join(tptp, "Problems", arg[:3], arg + ".p")

    if os.path.isdir(arg):
        for root, dirs, files in os.walk(arg):
            for file in files:
                ext = os.path.splitext(file)[1]
                if ext == ".p" and "^" not in file:
                    problems.append(os.path.join(root, file))
        continue
    if arg.endswith(".lst"):
        for s in open(arg):
            if "^" not in s:
                problems.append(s.rstrip())
        continue
    problems.append(arg)

# numbers larger than 2000 silently fail
sys.setrecursionlimit(2000)

ncalls = 0
nspecials = 0


def add_call():
    global ncalls
    ncalls += 1


def add_special():
    global nspecials
    nspecials += 1


class Inappropriate(Exception):
    def __init__(self):
        super().__init__("Inappropriate")


def read_tptp(filename, select=True):
    text = open(filename).read()

    # tokenizer
    ti = 0
    tok = ""

    def err(msg):
        raise Inappropriate()

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

    def args():
        expect("(")
        r = []
        if tok != ")":
            r.append(atomic_term())
            while tok == ",":
                lex()
                r.append(atomic_term())
        expect(")")
        return tuple(r)

    def atomic_term():
        o = tok

        # distinct object
        if o[0] == '"':
            lex()
            return o

        # number
        if o[0].isdigit() or o[0] == "-":
            lex()
            return o

        # variable
        if o[0].isupper():
            lex()
            return o

        # higher-order terms
        if tok == "!":
            raise Inappropriate()

        # function
        a = o
        lex()
        if tok == "(":
            s = args()
            if a.startswith("$"):
                add_special()
            else:
                add_call()
            return (a,) + s
        return a

    def infix_unary():
        a = atomic_term()
        o = tok
        if o == "=":
            lex()
            add_special()
            return "=", a, atomic_term()
        if o == "!=":
            lex()
            add_special()
            return "~", ("=", a, atomic_term())
        return a

    def var():
        o = tok
        if not o[0].isupper():
            err("expected variable")
        lex()
        if eat(":"):
            lex()
        return o

    def unitary_formula():
        o = tok
        if o == "(":
            lex()
            a = logic_formula()
            expect(")")
            return a
        if o == "~":
            lex()
            return "not", unitary_formula()
        if o in ("!", "?"):
            lex()

            # variables
            expect("[")
            v = []
            v.append(var())
            while tok == ",":
                lex()
                v.append(var())
            expect("]")

            # body
            expect(":")
            a = o, tuple(v), unitary_formula()
            return a
        return infix_unary()

    def logic_formula():
        a = unitary_formula()
        o = tok
        if o == "&":
            r = ["and", a]
            while eat("&"):
                r.append(unitary_formula())
            return tuple(r)
        if o == "|":
            r = ["or", a]
            while eat("|"):
                r.append(unitary_formula())
            return tuple(r)
        if o == "=>":
            lex()
            return o, a, unitary_formula()
        if o == "<=":
            lex()
            return o, unitary_formula(), a
        if o == "<=>":
            lex()
            return "eqv", a, unitary_formula()
        if o == "<~>":
            lex()
            return "not", ("eqv", a, unitary_formula())
        if o == "~&":
            lex()
            return "not", ("and", a, unitary_formula())
        if o == "~|":
            lex()
            return "not", ("or", a, unitary_formula())
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

    def annotated_formula():
        lex()
        expect("(")

        # name
        lex()
        expect(",")

        # role
        if tok == "type":
            while tok != ")":
                ignore()
            expect(")")
            expect(".")
            return
        lex()
        expect(",")

        # formula
        a = logic_formula()

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
        filename1 = tok
        lex()
        filename1 = filename1[1:-1]

        # select
        select1 = select
        if eat(","):
            expect("[")
            select1 = []
            while True:
                name = tok
                lex()
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
    while tok:
        if tok in ("cnf", "fof", "tff", "tcf"):
            annotated_formula()
            continue
        if tok == "include":
            include()
            continue
        err("unknown language")


for file in problems:
    # print(file)

    try:
        read_tptp(file)
    except Inappropriate as e:
        print(file, e)
    except RecursionError as e:
        print(file, e)


print(ncalls)
print(nspecials)
