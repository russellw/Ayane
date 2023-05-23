def is_header(s):
    if s.startswith("(set"):
        return 1
    if not s.startswith("("):
        return 1


def print_header(file):
    for s in open(file):
        if not is_header(s):
            break
        print(s, end="")


def get_expected(file):
    for s in open(file):
        if not is_header(s):
            break
        s = s.rstrip()
        if s == "(set-info :status unsat)":
            return "unsat"
        if s == "(set-info :status sat)":
            return "sat"
    raise Exception(file)


def check(r, expected):
    if not r:
        return
    if r == expected:
        return
    raise Exception(r + " != " + expected)
