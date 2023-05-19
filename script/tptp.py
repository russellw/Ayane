import os
import re


def get_problems(args):
    files = args.files
    if not files:
        files = ["tptp"]

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

    if args.random:
        random.shuffle(problems)
    if args.number:
        problems = problems[0 : args.number]
    return problems


def print_header(file):
    for s in open(file):
        s = s.rstrip()
        if s and not s.startswith("%"):
            break
        print(s)


def get_expected(file):
    for s in open(file):
        s = s.rstrip()
        if s and not s.startswith("%"):
            break
        m = re.match(r"% Status\s*:\s*(\w+)", s)
        if m:
            return m[1]
    raise Exception(file)


def check(r, expected):
    if not r:
        return

    if r == "sat":
        r = "Satisfiable"
    elif r == "unsat":
        r = "Unsatisfiable"

    if r == expected:
        return
    if r == "Satisfiable" and expected == "CounterSatisfiable":
        return
    if r == "Unsatisfiable" and expected in ("Theorem", "ContradictoryAxioms"):
        return

    raise Exception(r + " != " + expected)
