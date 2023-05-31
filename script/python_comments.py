import re

import common


def f(v):
    for i in range(len(v)):
        m = re.match(r"(\s*)# sort$", v[i], re.IGNORECASE)
        if m:
            v[i] = f"{m[1]}# SORT"
            continue

        m = re.match(r"(\s*)# todo:\s*(.*)", v[i], re.IGNORECASE)
        if m:
            v[i] = f"{m[1]}# TODO: {m[2]}"
            continue


common.modify_files(f, common.args_python_files())
