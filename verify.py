#!/usr/bin/python3
import argparse
import os
import re
import subprocess

parser = argparse.ArgumentParser(description="Run prover and verify proof")
parser.add_argument("files", nargs="+")
args = parser.parse_args()

subprocess.check_call(["make", "debug"])


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


formulas = []
formulad = {}


class Formula:
    def __init__(self, name, term, rl, fm, status):
        self.name = name
        self.term = term
        self.rl = rl
        self.fm = fm
        self.status = status
        formulas.append(self)
        formulad[name] = self

    def __repr__(self):
        return self.name


def getVars(s):
    r = set()
    i = 0
    while i < len(s):
        c = s[i]

        # space
        if c.isspace():
            i += 1
            continue

        # variable
        if c.isupper():
            j = i
            while s[i].isalnum() or s[i] == "_":
                i += 1
            r.add(s[j:i])
            continue

        # word
        if c.isalpha() or c == "$":
            i += 1
            while s[i].isalnum() or s[i] == "_":
                i += 1
            continue

        # quote
        if c in ("'", '"'):
            i += 1
            while s[i] != c:
                if s[i] == "\\":
                    i += 1
                i += 1
            i += 1
            continue

        # number
        if c.isdigit() or (c == "-" and s[i + 1].isdigit()):
            # integer part
            i += 1
            while s[i].isalnum():
                i += 1

            # rational
            if s[i] == "/":
                i += 1
                while s[i].isdigit():
                    i += 1

            # real
            else:
                if s[i] == ".":
                    i += 1
                    while s[i].isalnum():
                        i += 1
                if s[i - 1] in ("e", "E") and s[i] in ("+", "-"):
                    i += 1
                    while s[i].isdigit():
                        i += 1

            continue

        # punctuation
        if s[i : i + 3] in ("<=>", "<~>"):
            i += 3
            continue
        if s[i : i + 2] in ("!=", "=>", "<=", "~&", "~|"):
            i += 2
            continue
        i += 1
    return r


def quantify(s, f):
    r = getVars(s)
    if not r:
        f.write(s)
        return
    f.write("![")
    more = 0
    for x in r:
        if more:
            f.write(",")
        more = 1
        f.write(x)
    f.write("]:(")
    f.write(s)
    f.write(")")


def verify(filename):
    p = subprocess.Popen(
        ["./ayane", "-t", "60", filename],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = p.communicate()
    stdout = str(stdout, "utf-8")
    stderr = str(stderr, "utf-8")
    if stderr:
        print(stderr, end="")
    for s in stdout.splitlines():
        # definition formula
        m = re.match(
            r"tff\((\w+), plain, (.+), introduced\(definition\)\)\.",
            s,
        )
        if m:
            name = m[1]
            term = m[2]
            rl = "def"
            fm = []
            c = Formula(name, term, rl, fm, "axiom")
            continue

        # derived clause
        m = re.match(
            r"cnf\((\w+), plain, (.+), inference\((\w+),\[status\(thm\)\],\[(\w+)(,\w+)?\]\)\)\.",
            s,
        )
        if m:
            name = m[1]
            term = m[2]
            rl = m[3]
            fm = [m[4]]
            if m[5]:
                fm.append(m[5][1:])
            c = Formula(name, term, rl, fm, "thm")
            continue

    for c in formulas:
        fm = []
        for name in c.fm:
            if name not in formulad:
                raise Exception(name)
            fm.append(formulad[name])
        c.fm = fm

    for c in formulas:
        if c.status != "thm":
            continue
        if c.term == "$false":
            continue

        f = open("tmp.p", "w")
        for d in c.fm:
            f.write("tff(")
            f.write(d.name)
            f.write(",plain,")
            quantify(d.term, f)
            f.write(").\n")
        f.write("tff(")
        f.write(c.name)
        f.write(",plain,~(")
        quantify(c.term, f)
        f.write(")).\n")
        f.close()

        p = subprocess.Popen(
            ["bin/eprover", "tmp.p"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        stdout, stderr = p.communicate()
        stdout = str(stdout, "utf-8")
        stderr = str(stderr, "utf-8")
        if stderr:
            print(stderr, end="")
            exit(1)
        if p.returncode:
            raise Exception(str(p.returncode))
        if "Proof found" not in stdout:
            print(stdout)
            exit(1)


for arg in args.files:
    if not os.path.isfile(arg):
        for root, dirs, files in os.walk(arg):
            for filename in files:
                ext = os.path.splitext(filename)[1]
                if ext != ".p":
                    continue
                verify(os.path.join(root, filename))
        continue
    ext = os.path.splitext(arg)[1]
    if ext == ".lst":
        for filename in read_lines(arg):
            verify(filename)
        continue
    verify(arg)
