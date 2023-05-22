import datetime
import os
import re
import string
import subprocess
import sys
import time

import common
import tptp

args = common.args_problems()
codes = common.get_error_codes()
problems = tptp.get_problems(args)

start = time.time()
tried = 0
solved = 0
hardest = {}

try:
    for file in problems:
        print(file)
        tptp.print_header(file)
        expected = tptp.get_expected(file)

        cmd = "./ayane", "-t", str(args.time), file
        t = time.time()
        p = subprocess.run(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8"
        )
        t = time.time() - t
        print("%.3f seconds" % t)
        s = p.stdout
        print(s)

        code = codes.get(p.returncode, p.returncode)
        if code == "inappropriateError":
            continue
        tried += 1
        if code in (-14, 4294967282):
            continue
        if code:
            raise Exception(code)

        r = None
        if s.startswith("sat"):
            r = "sat"
        elif s.startswith("unsat"):
            r = "unsat"
        tptp.check(r, expected)

        if r:
            solved += 1
            if t > hardest.get(r, (0, 0))[1]:
                hardest[r] = file, t
except KeyboardInterrupt:
    print()

print("Total time")
t = time.time() - start
print(datetime.timedelta(seconds=t))
print()

if hardest:
    print("Hardest solved")
    if "sat" in hardest:
        print("sat\t%s\t%.3f" % hardest["sat"])
    if "unsat" in hardest:
        print("unsat\t%s\t%.3f" % hardest["unsat"])
    print()

if tried:
    print("Success rate")
    print(f"{solved}/{tried}")
    print("%f%%" % (float(solved) / tried * 100))
