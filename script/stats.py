import argparse
import os
import subprocess

import tptp

parser = argparse.ArgumentParser()
parser.add_argument(
    "-n", "--number", help="max number of problems to attempt", type=int
)
parser.add_argument(
    "-r", "--random", help="attempt problems in random order", action="store_true"
)
parser.add_argument("-s", "--seed", help="random number seed", type=int)
parser.add_argument(
    "-t", "--time", help="time limit per problem", type=float, default=60.0
)
parser.add_argument("files", nargs="*")
args = parser.parse_args()

if args.seed is not None:
    args.random = 1
    random.seed(args.seed)

problems = tptp.get_problems(args.files)
if args.random:
    random.shuffle(problems)
if args.number:
    problems = problems[0 : args.number]

m = {}
for file in problems:
    cmd = "./ayane", file
    p = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8"
    )
    code = p.returncode
    if code >= 1 << 31:
        code -= 1 << 32
    m[code] = m.get(code, 0) + 1
    if code:
        print(os.path.basename(file), code)
print(m)
