import os
import re

import common
def end(i):
    for j in range(i, len(v)):
        if re.match(r"\s*#\s*$", v[j]):
            return j
    raise Exception()


for file in common.args_python_files():
        v = open(file).readlines()
        o = v.copy()

        for i in range(len(v)):
            if re.match(r"\s*# SORT\s*$", v[i]):
                j = end(i + 1)
                w = v[i + 1 : j]
                w.sort()
                v[i + 1 : j] = w

        if v != o:
            print(file)
            open(file, "w", newline="\n").writelines(v)
