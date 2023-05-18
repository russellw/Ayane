import argparse
import filecmp
import glob
import os
import re
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("-s", "--save", help="save reference version", action="store_true")
args = parser.parse_args()

here = os.path.dirname(os.path.realpath(__file__))
src = os.path.join(here, "..", "src", "*.cc")

# compile the current version to assembly
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

# get the current list of source files
v = []
for f in glob.glob(src):
    f = os.path.basename(f)
    f = os.path.splitext(f)[0]
    v.append(f)

# save canonical assembly
if args.save:
    ver = "0"
else:
    ver = "1"
for f in v:
    cmd = "undname", f + ".asm"
    s = subprocess.check_output(cmd, encoding="utf-8")
    s = re.sub(r"A0x\w+", "_", s)
    open(f + ver + ".asm", "w", newline="\n").write(s)

if args.save:
    exit(0)

# compare the assembly output
for f in v:
    file0 = f + "0.asm"
    file1 = f + "1.asm"
    if not filecmp.cmp(file0, file1):
        print(f)
        os.remove(f + ".asm")
        os.system(f"fc {file0} {file1} > {f+'.diff.asm'}")
