import argparse
import os
import random
import re
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument(
    "-n", "--number", help="max number of problems to attempt", type=int
)
parser.add_argument(
    "-r", "--random", help="attempt problems in random order", action="store_true"
)
parser.add_argument("-s", "--seed", help="random number seed", type=int)
parser.add_argument(
    "-t", "--time", help="time limit per problem", type=float, default=60.0
)
parser.add_argument("files", nargs="*")
args = parser.parse_args()

if args.seed is not None:
    args.random = 1
    random.seed(args.seed)
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
if args.random:
    random.shuffle(problems)
if args.number:
    problems = problems[0 : args.number]

subprocess.check_call("make")


class Clause:
    def __init__(self, name, term, fm=None):
        self.name = name
        self.term = term
        self.fm = fm

    def __repr__(self):
        return self.name


def vars(s):
    i = 0
    r = set()
    while i < len(s):
        c = s[i]

        # variable
        if c.isupper():
            j = i
            while i < len(s) and (s[i].isalnum() or s[i] == "_"):
                i += 1
            r.add(s[j:i])
            continue

        # word
        if c.isalpha():
            i += 1
            while i < len(s) and (s[i].isalnum() or s[i] == "_"):
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

        # etc
        i += 1
    return r


def quantify(s, f):
    r = vars(s)
    if not r:
        f.write(s)
        return
    f.write("![")
    f.write(",".join(r))
    f.write("]:(")
    f.write(s)
    f.write(")")


for file in problems:
    # header
    expected = None
    for s in open(file):
        s = s.rstrip()
        if s and not s.startswith("%"):
            break
        print(s)
        if not expected:
            m = re.match(r"% Status\s+:\s+(\w+)\s*", s)
            if m:
                expected = m[1]
    assert expected

    # attempt proof
    cmd = "./ayane", "-t", str(args.time), file
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    out, err = p.communicate()
    out = str(out, "utf-8")
    print(out, end="")

    # clauses
    clauses = {}
    for s in out.splitlines():
        m = re.match(r"cnf\((\w+), plain, (.+), introduced\(definition\)\)\.$", s)
        if m:
            name = m[1]
            term = m[2]
            clauses[name] = Clause(name, term)
            continue

        m = re.match(
            r"cnf\((\w+), plain, (.+), inference\((cnf),\[status\(esa\)\],\[(\w+)\]\)\)\.$",
            s,
        )
        if m:
            name = m[1]
            term = m[2]
            clauses[name] = Clause(name, term)
            continue

        m = re.match(
            r"cnf\((\w+), plain, (.+), inference\(\w+,\[status\(thm\)\],\[(\w+)(,\w+)?\]\)\)\.$",
            s,
        )
        if m:
            name = m[1]
            term = m[2]
            fm = [m[3]]
            if m[4]:
                fm.append(m[4][1:])
            clauses[name] = Clause(name, term, fm)

    # resolve clause names
    for c in clauses.values():
        if c.fm is not None:
            c.fm = [clauses[name] for name in c.fm]

    # verify each clause
    for c in clauses.values():
        if c.fm is not None:
            print(c, c.term, c.fm)

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

            # should --auto-schedule be used here?
            cmd = "bin/eprover", "--auto", "tmp.p"
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
            out, err = p.communicate()
            out = str(out, "utf-8")
            if p.returncode:
                print(out, end="")
                raise Exception(str(p.returncode))
            m = re.search(r"SZS status (\w+)", out)
            if m and m[1] in ("Unsatisfiable", "Theorem"):
                continue
            raise Exception(out)

    print()
