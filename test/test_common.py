import re


def get_expected(file):
    for s in open(file).readlines():
        m = re.match(r"\S\s*(inappropriateError)\s+", s)
        if m:
            return m[1]

        m = re.match(r"\S\s*(inputError)\s+", s)
        if m:
            return m[1]

        m = re.match(r"\S\s*unsat\s+", s)
        if m:
            return 0

        m = re.match(r"\S\s*sat\s+", s)
        if m:
            return 1
    raise Exception(file)
