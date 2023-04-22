import argparse
import datetime
import os
import random
import re
import string
import subprocess
import sys
import time

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

success = [
    "CounterSatisfiable",
    "Satisfiable",
    "Theorem",
    "Unsatisfiable",
]

start = time.time()
tried = 0
solved = 0
hardest = {}

try:
    for file in problems:
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

        t = time.time()
        cmd = "R:/ayane.exe", "-t", str(args.time), file
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        out, err = p.communicate()
        out = str(out, "utf-8")
        t = time.time() - t

        print(out, end="")
        print("%0.3f" % t + " seconds")
        print()

        if p.returncode == 1:
            continue
        assert p.returncode in (0, 2)

        r = ""
        m = re.match("% SZS status (\w+)", out)
        if m:
            r = m[1]

        tried += 1
        if r in success:
            assert r == expected
            solved += 1
            if r in hardest:
                h = hardest[r]
                if t > h[1]:
                    hardest[r] = file, t
            else:
                hardest[r] = file, t
except KeyboardInterrupt:
    print()

for r in success:
    if r in hardest:
        file, t = hardest[r]
        print("Hardest " + r)
        print(file)
        print("%0.3f seconds" % t)
        print()

if tried:
    print("Success rate")
    print(str(solved) + "/" + str(tried))
    print(str(float(solved) / tried * 100) + "%")
    print()

print("Total time")
t = time.time() - start
print(datetime.timedelta(seconds=t))
