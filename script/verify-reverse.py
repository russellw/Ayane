import argparse
import os
import random
import re
import subprocess
import time

parser = argparse.ArgumentParser()
parser.add_argument(
    "-n", "--number", help="max number of problems to attempt", type=int
)
parser.add_argument("-l", "--log", help="write output to log file", action="store_true")
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
                if ext == ".p" and "^" not in file and "_" not in file:
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


def lex(s):
    v = []
    i = 0
    while i < len(s):
        j = i
        c = s[i]
        if c.isspace():
            i += 1
            continue
        elif c.isalnum():
            while i < len(s) and (s[i].isalnum() or s[i] == "_"):
                i += 1
        elif c in ("'", '"'):
            i += 1
            while s[i] != c:
                if s[i] == "\\":
                    i += 1
                i += 1
            i += 1
        else:
            i += 1
        v.append(s[j:i])
    return v


def parse(v):
    depth = 1
    i = 0
    while 1:
        t = v[i]
        if t == "(":
            depth += 1
        elif t == ")":
            depth -= 1
            if depth == 0:
                return i
        i += 1


class Clause:
    def __init__(self, name, term, fm):
        self.name = name
        self.term = term
        self.fm = fm

    def __repr__(self):
        return self.name


def quantify(v):
    r = set([x for x in v if x[0].isupper()])
    s = "".join(v)
    if not r:
        return s
    return f"![{','.join(r)}]: ({s})"


for file in problems:
    print(os.path.basename(file), end="\t", flush=True)

    # --auto makes a big difference to performance
    # don't use --auto-schedule
    # for some reason, it breaks the subprocess timeout feature
    cmd = "bin/eprover", "--auto", "-p", file

    start1 = time.time()
    try:
        p = subprocess.run(
            cmd, capture_output=True, encoding="utf-8", timeout=args.time
        )
    except subprocess.TimeoutExpired:
        print()
        continue
    if p.returncode not in (0, 1, 9):
        raise Exception(p.returncode)

    m = re.search(r"SZS status (\w+)", p.stdout)
    print(m[1], end="\t", flush=True)

    if args.log:
        open("a.log", "w", newline="\n").write(p.stdout)

    # clauses
    clauses = {}
    for s in p.stdout.splitlines():
        m = re.match(r"cnf\((\w+), \w+, \((.+)", s)
        if m:
            name = m[1]
            v = lex(m[2])
            i = parse(v)
            term = v[:i]
            fm = [clauses[t] for t in v[i:] if t in clauses]
            clauses[name] = Clause(name, term, fm)

    # verify each clause
    for c in clauses.values():
        if c.fm:
            cmd = "./ayane", "-tptp"

            v = []
            for d in c.fm:
                v.append(f"tff({d.name}, plain, {quantify(d.term)}).")
            v.append(f"tff({c.name}, plain, ~({quantify(c.term)})).")
            s = "\n".join(v)

            if args.log:
                open("a.p", "w", newline="\n").write(s)

            s = subprocess.check_output(cmd, input=s, encoding="utf-8")

            m = re.search(r"SZS status (\w+)", s)
            if not (m and m[1] in ("Unsatisfiable", "Theorem")):
                raise Exception(s)

            print(".", end="", flush=True)

    # total time spent on this problem
    print(" %0.3f" % (time.time() - start1))
