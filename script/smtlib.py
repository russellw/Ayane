import os
import random
import re

supported_logics = (
    "QF_UF",
    "QF_LIA",
    "QF_NIA",
    "QF_LRA",
    "LRA",
    "QF_IDL",
    "QF_RDL",
    "QF_UFIDL",
)
unsupported_logics = (
    "QF_AUFLIA",
    "AUFLIA",
    "AUFLIRA",
    "AUFNIRA",
    "QF_BV",
    "QF_AX",
    "QF_ABV",
    "QF_AUFBV",
    "ABVFP",
    "ALIA",
)


def get_problems(args):
    problems = []
    for root, dirs, files in os.walk("C:\\smtlib"):
        for file in files:
            ext = os.path.splitext(file)[1]
            if ext == ".smt2":
                problems.append(os.path.join(root, file))

    if args.random:
        random.shuffle(problems)
    if args.number:
        problems = problems[0 : args.number]
    return problems


def print_header(file):
    for s in open(file):
        if s.startswith("(declare-fun"):
            break
        print(s, end="")


def get_logic(file):
    for s in open(file):
        m = re.match(r"\(set-logic (\w+)\)", s)
        if m:
            return m[1]
    raise Exception(file)


def get_expected(file):
    for s in open(file):
        m = re.match(r"\(set-info :status (\w+)\)", s)
        if m:
            return m[1]
    raise Exception(file)


def read(file):
    text = open(filename).read()

    # tokenizer
    ti = 0
    tok = ""

    def err(msg):
        line = 1
        for i in range(ti):
            if text[i] == "\n":
                line += 1
        raise Exception(f"{file}:{line}: {repr(tok)}: {msg}")

    def issym(c):
        if c.isalnum():
            return 1
        return c in "~!@$%^&*_-+=<>.?/"

    def lex():
        nonlocal ti
        nonlocal tok
        while ti < len(text):
            i = ti
            c = text[ti]

            # space
            if c.isspace():
                ti += 1
                continue

            # line comment
            if c == ";":
                while text[ti] != "\n":
                    ti += 1
                continue

            # symbol, number or keyword
            if issym(c) or c in "#:":
                ti += 1
                while issym(text[ti]):
                    ti += 1
                tok = text[i:ti]
                return

            # quote
            if c in ("|", '"'):
                ti += 1
                while text[ti] != c:
                    if text[ti] == "\\":
                        ti += 1
                    ti += 1
                ti += 1
                tok = text[i:ti]
                return

            # punctuation
            tok = c
            ti += 1
            return

        # end of file
        tok = None

    def lex1():
        k = tok
        lex()
        return k

    # parser
    def eat(k):
        if tok == k:
            lex()
            return True

    def expect(k):
        if not eat(k):
            err(f"expected '{k}'")

    def expr():
        if eat("("):
            v = []
            while not eat(")"):
                v.append(expr())
            return v
        if tok:
            return lex1()
        err("unclosed '('")

    # top level
    v = []
    while tok:
        v.append(expr())
    return v


def check(r, expected):
    if not r:
        return
    if r == expected:
        return
    raise Exception(r + " != " + expected)
