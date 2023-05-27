import os
import subprocess
import time

import tptp

import common

args = common.args_problems()
codes = common.get_error_codes()
problems = tptp.get_problems(args)

m = {}
for file in problems:
    print(os.path.basename(file), end="\t")

    cmd = "./ayane", file
    start = time.time()
    p = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8"
    )
    print("%.3f" % (time.time() - start), end="\t")

    code = p.returncode
    code = codes.get(code, code)
    print(code, end="")
    if code not in (0, "inappropriateError"):
        raise Exception(code)
    m[code] = m.get(code, 0) + 1

    print()
print(m)
