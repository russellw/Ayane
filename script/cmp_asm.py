import filecmp
import glob
import os
import subprocess
import tempfile


def build(src):
    cmd = (
        "C:/Program Files/LLVM/bin/clang-cl",
        "-Fa",
        "-IC:/mpir",
        "-O2",
        "-Wimplicit-fallthrough",
        "-Wno-assume",
        "-Wno-deprecated-declarations",
        "-Wno-switch",
        "-c",
        "-std:c++17",
        src,
    )
    subprocess.check_call(cmd)


def simplify(file):
    cmd = "undname", file
    s = subprocess.check_output(cmd, encoding="utf-8")
    open(file, "w", newline="\n").write(s)


here = os.path.dirname(os.path.realpath(__file__))

# get the current list of source files
v = []
for s in glob.glob(os.path.join(here, "..", "src", "*.cc")):
    s = os.path.basename(s)
    s = os.path.splitext(s)[0]
    v.append(s)

# compile the current version to assembly
build(os.path.join(here, "..", "src", "*.cc"))
for s in v:
    simplify(s + ".asm")
    os.replace(s + ".asm", s + "1.asm")

# compile the previous version to assembly
build(os.path.join(tempfile.gettempdir(), "0", "*.cc"))
for s in v:
    simplify(s + ".asm")
    os.replace(s + ".asm", s + "0.asm")

# get the object files out of the way
for s in v:
    os.remove(s + ".obj")

# compare the assembly output
for s in v:
    file0 = s + "0.asm"
    file1 = s + "1.asm"
    if not filecmp.cmp(file0, file1):
        print(s)
        os.system(f"fc {file0} {file1} > {s+'-diff.asm'}")
