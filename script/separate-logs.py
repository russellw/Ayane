import argparse
import os
import re
import sys

parser = argparse.ArgumentParser()
parser.add_argument("files", nargs="+")
args = parser.parse_args()

os.makedirs("solved", exist_ok=True)
os.makedirs("unsolved", exist_ok=True)


def solved(file):
    for s in open(file):
        m = re.search(r"SZS status (\w+)", s)
        if m:
            return m[1] in (
                "ContradictoryAxioms",
                "CounterSatisfiable",
                "Satisfiable",
                "Theorem",
                "Unsatisfiable",
            )


def replace_dir(file, d):
    file = os.path.basename(file)
    return os.path.join(d, file)


def do(file):
    d = "solved" if solved(file) else "unsolved"
    os.rename(file, replace_dir(file, d))


for arg in args.files:
    if os.path.isdir(arg):
        for root, dirs, files in os.walk(arg):
            for file in files:
                ext = os.path.splitext(file)[1]
                if ext == ".log":
                    do(os.path.join(root, file))
        continue
    ext = os.path.splitext(arg)[1]
    if ext == ".lst":
        for file in open(arg):
            do(file.strip())
        continue
    do(arg)
