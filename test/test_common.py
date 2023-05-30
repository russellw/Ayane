import os
import re


# SORTF
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


def get_logic(file):
    ext = os.path.splitext(file)[1]
    assert ext == ".smt2"
    for s in open(file).readlines():
        m = re.match(r"\(set-logic\s+(\w+)\)", s)
        if m:
            return m[1]


def get_p(file):
    ext = os.path.splitext(file)[1]
    assert ext == ".cnf"
    for s in open(file).readlines():
        m = re.match(r"p\s+cnf\s+(\d+)\s+(\d+)", s)
        if m:
            return m[1], m[2]
