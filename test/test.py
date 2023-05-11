import os
import re
import subprocess

here = os.path.dirname(os.path.realpath(__file__))
src = os.path.join(here, "..", "src", "*.cc")

cmd = (
    "cl",
    "/DDBG",
    "/Feayane",
    "/IC:/mpir",
    "/MP",
    "/MTd",
    "/WX",
    "/Zi",
    "/std:c++17",
    src,
    "C:/mpir/debug.lib",
    "dbghelp.lib",
)
subprocess.check_call(cmd)
print()

problems = []
for root, dirs, files in os.walk(here):
    for file in files:
        ext = os.path.splitext(file)[1]
        if ext in (".p", ".cnf"):
            problems.append(os.path.join(root, file))

for file in problems:
    print(file)

    e = None
    for s in open(file).readlines():
        m = re.match(r"\S\s*unsat\s+", s)
        if m:
            e = 0
            break

        m = re.match(r"\S\s*sat\s+", s)
        if m:
            e = 1
            break
    assert e is not None

    cmd = "ayane", "-t3", file
    s = subprocess.check_output(cmd, encoding="utf-8")
    r = None
    if "unsat" in s:
        r = 0
    elif "sat" in s:
        r = 1
    else:
        raise Exception(s)
    if r != e:
        raise Exception(s)
