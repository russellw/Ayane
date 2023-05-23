import datetime
import os
import subprocess
import time

import dimacs

start = time.time()
solved = 0
hardest = {}

try:
    for file in dimacs.satlib_problems:
        file = os.path.join("C:\\satlib", file)
        print(file)
        dimacs.print_header(file)
        expected = dimacs.get_expected(file)

        cmd = "./ayane", file
        t = time.time()
        p = subprocess.run(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8"
        )
        t = time.time() - t
        print("%.3f seconds" % t)
        s = p.stdout
        print(s)

        code = p.returncode
        if code:
            raise Exception(code)

        if s.startswith("sat"):
            r = "sat"
        elif s.startswith("unsat"):
            r = "unsat"
        else:
            raise Exception()
        dimacs.check(r, expected)

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

print(solved)
