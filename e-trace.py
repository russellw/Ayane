import argparse
import os
import re
import subprocess

parser = argparse.ArgumentParser(
    description="Run the E prover, tracing generated clauses"
)
parser.add_argument("files", nargs="+")
args = parser.parse_args()

os.makedirs("logs-e", exist_ok=True)


def replace_dir(file, d):
    file = os.path.basename(file)
    return os.path.join(d, file)


def replace_ext(file, ext):
    file = os.path.splitext(file)[0]
    return file + ext


def do(file):
    print(file)
    # --auto makes a big difference to performance
    # don't use --auto-schedule
    # for some reason, it breaks the timeout feature
    cmd = "E/PROVER/eprover", "--auto", "-l", "1", file
    file = replace_dir(file, "logs-e")
    file = replace_ext(file, ".log")
    out = open(file, "w")
    try:
        p = subprocess.run(cmd, stdout=out, timeout=60)
    except subprocess.TimeoutExpired:
        pass


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
