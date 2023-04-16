# the #! line has been intentionally omitted here
# as this script requires subprocess capture_output
# which does not work with the version of /usr/bin/python3 in WSL at this time
# run this script with 'python ...'
import argparse
import os
import re
import subprocess

parser = argparse.ArgumentParser(description="Run the E prover on a batch of problems")
parser.add_argument("files", nargs="+")
args = parser.parse_args()


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def difficulty(f):
    xs = read_lines(f)
    for x in xs:
        m = re.match(r"% Rating   : (\d+\.\d+)", x)
        if m:
            return m[1]
    return "?"


def do_file(f):
    print(f, end=",", flush=True)
    print(difficulty(f), end=",", flush=True)
    cmd = ["bin/eprover", "-p", f]
    try:
        p = subprocess.run(
            cmd,
            capture_output=True,
            encoding="utf-8",
            timeout=3,
            check=True,
        )
        print(len(p.stdout.splitlines()), end=",", flush=True)
        if "Proof found" in p.stdout:
            r = "Unsatisfiable"
        elif "No proof found":
            r = "Satisfiable"
        else:
            print(p.stdout)
            exit(1)
    except subprocess.TimeoutExpired:
        print(0, end=",", flush=True)
        r = "Timeout"
    print(r, flush=True)


for arg in args.files:
    if not os.path.isfile(arg):
        for root, dirs, files in os.walk(arg):
            for filename in files:
                ext = os.path.splitext(filename)[1]
                if ext != ".p":
                    continue
                do_file(os.path.join(root, filename))
        continue
    ext = os.path.splitext(arg)[1]
    if ext == ".lst":
        for filename in read_lines(arg):
            do_file(filename)
        continue
    do_file(arg)
