import argparse
import os
import re
import subprocess
import sys

import test_common

parser = argparse.ArgumentParser()
parser.add_argument("files", nargs="*")
args = parser.parse_args()

here = os.path.dirname(os.path.realpath(__file__))

codes = {}
for s in open(os.path.join(here, "..", "src", "etc.h")).readlines():
    m = re.match(r"const int (\w+Error) = (\d+);", s)
    if m:
        codes[int(m[2])] = m[1]

problems = []
if args.files:
    for file in args.files:
        problems.append(os.path.join(here, file))
else:
    for root, dirs, files in os.walk(here):
        for file in files:
            ext = os.path.splitext(file)[1]
            if ext in (".p", ".cnf", ".smt2"):
                problems.append(os.path.join(root, file))


def err():
    print(s, end="")
    raise Exception(str(code))


for file in problems:
    print(file)
    e = test_common.get_expected(file)

    cmd = "./ayane", "-t3", file
    p = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8"
    )
    code = p.returncode
    code = codes.get(code, code)
    s = p.stdout

    if code in ("inappropriateError", "inputError"):
        r = code
    elif code:
        err()
    elif "unsat" in s:
        r = 0
    elif "sat" in s:
        r = 1
    else:
        err()

    if r != e:
        err()
