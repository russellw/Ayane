#!/usr/bin/python
import datetime
import itertools
import optparse
import os
import random
import re
import string
import subprocess
import sys
import time


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


parser = optparse.OptionParser(
    usage="Usage: %prog [options] [files]", description="Process a batch of problems"
)
parser.add_option("-f", "--filter", dest="regex", help="filter filenames")
parser.add_option(
    "-n", "--number", type="int", help="max number of problems to attempt"
)
parser.add_option(
    "-C",
    "--iters",
    type="int",
    help="iterations",
    default=1000,
)
parser.add_option(
    "-r", "--random", action="store_true", help="attempt problems in random order"
)
parser.add_option("-s", "--seed", help="random number seed")
options, args = parser.parse_args()

if not args:
    exit(0)

if args == ["."]:
    tptp = os.getenv("TPTP")
    if not tptp:
        print("TPTP environment variable not set")
        exit(1)
    args = [tptp]

# skip other files and higher-order problems
def ok(filename):
    if os.path.splitext(filename)[1] != ".p":
        return 0
    for c in ["^"]:
        if filename.find(c) >= 0:
            return 0
    return 1


# directory names are ok
problems = []
for arg in args:
    if os.path.isfile(arg):
        if arg.endswith(".lst"):
            problems.extend(read_lines(arg))
            continue
        problems.append(arg)
        continue
    for root, dirs, files in os.walk(arg):
        for filename in files:
            filename = os.path.join(root, filename)
            if ok(filename):
                problems.append(filename)

# filter filenames
if options.regex:
    problems = [s for s in problems if re.match(options.regex, s)]

# order
if options.seed:
    options.random = 1
    random.seed(options.seed)
if options.random:
    random.shuffle(problems)
else:
    problems.sort()

# max number of problems to attempt
problems = problems[0 : options.number]

# copy output to log file
lname = datetime.datetime.now().strftime("%Y-%m-%d %H%M%S")
log = open(lname, "w")


def pr(s):
    sys.stdout.write(s)
    log.write(s)


# attempt problems
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
    for filename in problems:
        expected = None

        h = read_lines(filename)
        i = 1
        while i < len(h) and not h[i].startswith("%--"):
            i += 1
        i = min(i + 1, len(h))
        h = h[:i]
        for s in h:
            pr(s + "\n")
            if not expected:
                m = re.match(r"% Status\s+:\s+(\w+)\s*", s)
                if m:
                    expected = m.group(1)

        t = time.time()
        p = subprocess.Popen(
            ["./ayane", "-C" + str(options.iters), filename],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        out, err = p.communicate()
        out = str(out, "utf-8")
        err = str(err, "utf-8")
        t = time.time() - t

        if out:
            pr(out)
        if err:
            pr(err)
        pr("%0.3f" % t + " seconds\n")
        pr("\n")

        if p.returncode not in (0, 1, -14):
            break

        r = ""
        m = re.match("% SZS status (\w+)", out)
        if m:
            r = m.group(1)
            if r not in success:
                r = ""

        tried += 1
        if r in success:
            if expected and r != expected:
                pr("Expected " + expected + "\n")
                exit(1)
            solved += 1

        if r in [""] + success:
            if r in hardest:
                h = hardest[r]
                if t > h[1]:
                    hardest[r] = filename, t
            else:
                hardest[r] = filename, t
except KeyboardInterrupt:
    pr("\n")

for r in [""] + success:
    if r in hardest:
        filename, t = hardest[r]
        pr("Hardest " + r + "\n")
        pr(filename + "\n")
        pr("%0.3f seconds\n" % t)
        pr("\n")

if tried:
    pr("Success rate\n")
    pr(str(solved) + "/" + str(tried) + "\n")
    pr(str(float(solved) / tried * 100) + "%\n")
    pr("\n")

pr("Total time\n")
t = time.time() - start
pr(str(datetime.timedelta(seconds=t)) + "\n")
