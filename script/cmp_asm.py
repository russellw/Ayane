import glob
import os
import subprocess
import tempfile

v = []
for s in glob.glob(os.path.join(tempfile.gettempdir(), "0", "*.cc")):
    s = os.path.basename(s)
    s = os.path.splitext(s)[0]
    v.append(s)


def build(ver):
    cmd = (
        "cl",
        "/Fa",
        "/IC:\mpir",
        "/MP",
        "/O2",
        "/c",
        "/std:c++17",
        os.path.join(tempfile.gettempdir(), ver, "*.cc"),
    )
    subprocess.check_call(cmd)
    for s in v:
        os.replace(s + ".asm", s + ver + ".asm")


build("0")
build("1")
