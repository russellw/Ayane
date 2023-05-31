import re

import common


# SORTF
def f(v):
    i = 0
    while i < len(v):
        if not iscase(v, i):
            i += 1
            continue

        j = i + 1
        while iscase(v, j):
            j += 1
        w = v[i:j]

        brace = 0
        if w[-1].endswith(" {"):
            w[-1] = w[-1][:-2]
            brace = 1

        w.sort()

        if brace:
            w[-1] += " {"

        v[i:j] = w
        i += len(w)


def iscase(v, i):
    if re.match(r"\s*case .*:", v[i]):
        return 1
    if re.match(r"\s*default:", v[i]):
        return 1


common.modify_files(f, common.args_c_files())
