# this script requires subprocess capture_output
# which does not work with the version of /usr/bin/python3 in WSL at this time
# run with e.g.
# ./anaconda/bin/python ...
import argparse
import os
import re
import subprocess

parser = argparse.ArgumentParser(description="Run the E prover on a batch of problems")
parser.add_argument("files", nargs="+")
args = parser.parse_args()


def difficulty(file):
    for s in open(file):
        m = re.match(r"% Rating   : (\d+\.\d+)", s)
        if m:
            return m[1]
    return "?"


def do(file):
    print(file, end=",", flush=True)
    print(difficulty(file), end=",", flush=True)
    cmd = "E/PROVER/eprover", "-p", file
    try:
        p = subprocess.run(
            cmd, capture_output=True, check=True, encoding="utf-8", timeout=3
        )
        print(len(p.stdout.splitlines()), end=",", flush=True)
        if "Proof found" in p.stdout:
            r = "Unsatisfiable"
        elif "No proof found":
            r = "Satisfiable"
        else:
            raise Exception(p.stdout)
    except subprocess.TimeoutExpired:
        print(0, end=",", flush=True)
        r = "Timeout"
    print(r, flush=True)


for arg in args.files:
    if os.path.isdir(arg):
        for root, dirs, files in os.walk(arg):
            for file in files:
                ext = os.path.splitext(file)[1]
                if ext != ".p":
                    continue
                do(os.path.join(root, file))
        continue
    ext = os.path.splitext(arg)[1]
    if ext == ".lst":
        for file in open(arg):
            do(file.strip())
        continue
    do(arg)
