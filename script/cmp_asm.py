import glob
import os
import subprocess
import tempfile


def build(src):
    cmd = (
        "cl",
        "/Fa",
        "/IC:\mpir",
        "/MP",
        "/O2",
        "/c",
        "/std:c++17",
        src,
    )
    subprocess.check_call(cmd)


def count_lines(file):
    return len(open(file).readlines())


here = os.path.dirname(os.path.realpath(__file__))

# first use clang to check for errors, as its diagnostics are slightly better
subprocess.check_call(os.path.join(here, "build_debug_clang.bat"))

# get the current list of source files
v = []
for s in glob.glob(os.path.join(here, "..", "src", "*.cc")):
    s = os.path.basename(s)
    s = os.path.splitext(s)[0]
    v.append(s)

# compile the current version to assembly
build(os.path.join(here, "..", "src", "*.cc"))
for s in v:
    os.replace(s + ".asm", s + "1.asm")

# compile the previous version to assembly
build(os.path.join(tempfile.gettempdir(), "0", "*.cc"))
for s in v:
    os.replace(s + ".asm", s + "0.asm")

# compare the assembly output
for s in v:
    s0 = s + "0.asm"
    s1 = s + "1.asm"
    n0 = count_lines(s0)
    n1 = count_lines(s1)
    if n0 != n1:
        print(s, n0, n1)
        os.system(f"fc {s0} {s1} > {s+'-diff.asm'}")
