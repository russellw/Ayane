import argparse
import os
import re
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument("files", nargs="*")
args = parser.parse_args()

here = os.path.dirname(os.path.realpath(__file__))

if sys.platform == "win32":
    src = os.path.join(here, "..", "src", "*.cc")

    # Build with two different compilers to get two opinions on error checking
    # first with clang, which has slightly better diagnostics
    cmd = (
        "C:/Program Files/LLVM/bin/clang-cl",
        "/DDBG",
        "/Feayane",
        "/IC:/mpir",
        "/MTd",
        "/WX",
        "/Zi",
        "/std:c++17",
        "-Wimplicit-fallthrough",
        "-Wno-deprecated-declarations",
        "-Wno-switch",
        "-ferror-limit=1",
        src,
        "C:/mpir/debug.lib",
        "dbghelp.lib",
    )
    subprocess.check_call(cmd)

    # then with Microsoft C++
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
else:
    subprocess.check_call("make")

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

    cmd = "./ayane", "-t3", file
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
