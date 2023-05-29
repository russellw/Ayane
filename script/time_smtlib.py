import datetime
import subprocess
import time

import common
import smtlib

args = common.args_problems()
codes = common.get_error_codes()
problems = smtlib.get_problems(args)

for file in problems:
    expected = smtlib.get_expected(file)

    cmd = "./ayane", "-t", str(args.time), file
    t = time.time()
    p = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8"
    )
    t = time.time() - t
    if t > 30:
        print("%s\t%.3f seconds" % (file, t))
    s = p.stdout

    code = codes.get(p.returncode, p.returncode)
    if code == "inappropriateError":
        continue
    tried += 1
    if code in (-14, 4294967282):
        continue
    if code:
        raise Exception(code)

    r = None
    if s.startswith("sat"):
        r = "sat"
    elif s.startswith("unsat"):
        r = "unsat"
    smtlib.check(r, expected)
