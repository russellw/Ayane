import os
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
            j = end(v, i + 1)
            w = v[i + 1 : j]
            if "" in w:
                raise Exception(i)
            w.sort()
            v[i + 1 : j] = w


common.modify_files(f, common.args_python_files())
