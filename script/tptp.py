import os
import re


def get_problems(files):
    if not files:
        files = ["tptp_dir"]

    tptp_dir = os.getenv("TPTP")
    if not tptp_dir:
        raise Exception("TPTP environment variable not set")

    problems = []
    for arg in files:
        if arg.lower() == "tptp":
            arg = tptp_dir
        elif re.match(r"[A-Za-z][A-Za-z][A-Za-z]$", arg):
            arg = arg.upper()
            arg = os.path.join(tptp_dir, "Problems", arg)
        elif re.match(r"[A-Za-z][A-Za-z][A-Za-z]\d\d\d.\d+$", arg):
            arg = arg.upper()
            arg = os.path.join(tptp_dir, "Problems", arg[:3], arg + ".p")

        if os.path.isdir(arg):
            for root, dirs, files in os.walk(arg):
                for file in files:
                    ext = os.path.splitext(file)[1]
                    if ext == ".p" and "^" not in file:
                        problems.append(os.path.join(root, file))
            continue
        if arg.endswith(".lst"):
            for s in open(arg):
                if "^" not in s:
                    problems.append(s.rstrip())
            continue
        problems.append(arg)
    return problems
