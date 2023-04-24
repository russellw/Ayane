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


def quantify(s):
    r = vars(s)
    if not r:
        return s
    return f"![{','.join(r)}]: ({s})"


for file in problems:
    print(os.path.basename(file), end="\t", flush=True)

    # --auto makes a big difference to performance
    # don't use --auto-schedule
    # for some reason, it breaks the subprocess timeout feature
    cmd = "bin/eprover", "--auto", "-p", file

    t = time.time()
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
            continue

        if s.startswith("cnf"):
            raise Exception(s)

    # resolve clause names
    for c in clauses.values():
        if c.fm is not None:
            c.fm = [clauses[name] for name in c.fm]

    # verify each clause
    n = 0
    for c in clauses.values():
        if c.fm is not None:
            cmd = "./ayane"

            v = []
            for d in c.fm:
                v.append(f"tff({d.name}, plain, {quantify(d.term)}).")
            v.append(f"tff({c.name}, plain, ~({quantify(c.term)})).")

            s = subprocess.check_output(cmd, input="\n".join(v), encoding="utf-8")

            m = re.search(r"SZS status (\w+)", s)
            if not (m and m[1] in ("Unsatisfiable", "Theorem")):
                raise Exception(s)

            n += 1
    print(n, end="\t")

    # total time spent on this problem
    print("%0.3f" % (time.time() - t))
