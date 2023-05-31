import re

import common


def end(v, i):
    for j in range(i, len(v)):
        if re.match(r"\s*#\s*$", v[j]):
            return j
    raise Exception(i)


def f(v):
    for i in range(len(v)):
        if re.match(r"\s*# SORT\s*$", v[i]):
            i += 1
            j = end(v, i)
            r = v[i:j]
            if "" in r:
                raise Exception(i)
            r.sort()
            v[i:j] = r


common.modify_files(f, common.args_python_files())
