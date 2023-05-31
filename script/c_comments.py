import re

import common


def special(s):
    m = re.match(r"(\s*)// sort$", s, re.IGNORECASE)
    if m:
        return f"{m[1]}// SORT"

    m = re.match(r"(\s*)// sortf$", s, re.IGNORECASE)
    if m:
        return f"{m[1]}// SORTF"

    m = re.match(r"(\s*)# todo:\s*(.*)", s, re.IGNORECASE)
    if m:
        return f"{m[1]}// TODO: {m[2]}"


def f(v):
    for i in range(len(v)):
        s = special(v[i])
        if s:
            v[i] = s
            continue


common.modify_files(f, common.args_c_files())
