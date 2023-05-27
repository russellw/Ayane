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

problems = []
if args.files:
    for file in args.files:
        problems.append(os.path.join(here, file))
else:
    for root, dirs, files in os.walk(here):
        for file in files:
            ext = os.path.splitext(file)[1]
            if ext in (".smt2",):
                problems.append(os.path.join(root, file))


def err():
    print(s, end="")
    raise Exception(str(code))


for file in problems:
    e = test_common.get_expected(file)
    if e in ("inappropriateError", "inputError"):
        continue

    L = test_common.get_logic(file)
    if L not in (
        "QF_UF",
        "QF_RDL",
        "QF_IDL",
        "QF_LRA",
        "QF_LIA",
        "QF_UFLRA",
        "QF_UFLIA",
    ):
        continue

    print(file, end="\t")

    cmd = os.path.join(here, "..", "bin", "opensmt"), file
    p = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8"
    )
    code = p.returncode
    s = p.stdout.lower()

    print()

    if code:
        err()
    elif "unsat" in s:
        r = 0
    elif "sat" in s:
        r = 1
    else:
        err()

    if r != e:
        err()
