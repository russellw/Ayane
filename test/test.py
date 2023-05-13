import argparse
import os
import re
import subprocess
import sys

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
            if ext in (".p", ".cnf"):
                problems.append(os.path.join(root, file))


def err():
    print(s, end="")
    raise Exception(str(code))


for file in problems:
    print(file)

    e = None
    for s in open(file).readlines():
        m = re.match(r"\S\s*err\s+", s)
        if m:
            e = -1
            break

        m = re.match(r"\S\s*unsat\s+", s)
        if m:
            e = 0
            break

        m = re.match(r"\S\s*sat\s+", s)
        if m:
            e = 1
            break
    assert e is not None

    cmd = "./ayane", "-t3", file
    p = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8"
    )
    code = p.returncode
    if code >= 1 << 31:
        code -= 1 << 32
    s = p.stdout
    if code not in (0, -1):
        err()

    if code == -1:
        r = -1
    elif "unsat" in s:
        r = 0
    elif "sat" in s:
        r = 1
    else:
        err()

    if r != e:
        err()
