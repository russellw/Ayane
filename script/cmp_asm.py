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


v = []
for s in glob.glob(os.path.join(tempfile.gettempdir(), "0", "*.cc")):
    s = os.path.basename(s)
    s = os.path.splitext(s)[0]
    v.append(s)

build(os.path.join(tempfile.gettempdir(), "0", "*.cc"))
for s in v:
    os.replace(s + ".asm", s + "0.asm")

here = os.path.dirname(os.path.realpath(__file__))
build(os.path.join(here, "..", "src", "*.cc"))

for s in v:
    s0 = s + "0.asm"
    s1 = s + ".asm"
    n0 = count_lines(s0)
    n1 = count_lines(s1)
    if n0 != n1:
        print(s, n0, n1)
        os.system(f"fc {s0} {s1} > {s+'-diff.asm'}")
