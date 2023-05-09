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


def difficulty(file):
    for s in open(file):
        m = re.match(r"% Rating   : (\d+\.\d+)", s)
        if m:
            return m[1]
    return "?"


for file in problems:
    print(os.path.basename(file), end="\t")
    print(difficulty(file), end="\t", flush=True)

    # --auto makes a big difference to performance
    # don't use --auto-schedule
    # for some reason, it breaks the subprocess timeout feature
    cmd = "bin/eprover", "--auto", "-p", file

    t = time.time()
    try:
        p = subprocess.run(
            cmd, capture_output=True, encoding="utf-8", timeout=args.time
        )
        # if p.returncode == 3:
        #    print()
        #    continue
        if p.returncode not in (0, 1, 9):
            raise Exception(p.returncode)

        print("%0.3f" % (time.time() - t), end="\t")
        print(len(p.stdout.splitlines()), end="\t")
        m = re.search(r"SZS status (\w+)", p.stdout)
        r = m[1]
    except subprocess.TimeoutExpired:
        print("%0.3f" % (time.time() - t), end="\t")
        print(0, end="\t")
        r = "Timeout"
    print(r)
