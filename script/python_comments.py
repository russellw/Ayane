import os
import re

import common


def f(v):
    for i in range(len(v)):
        m = re.match(r"(\s*)# sort$", v[i])
        if m:
            v[i] = f"{m[1]}# SORT"
            continue

        m = re.match(r"(\s*)# sortf$", v[i])
        if m:
            v[i] = f"{m[1]}# SORTF"
            continue

        m = re.match(r"(\s*)# todo: (.*)", v[i])
        if m:
            v[i] = f"{m[1]}# TODO: {m[2]}"
            continue

        m = re.match(r"(\s*)# todo:(.*)", v[i])
        if m:
            v[i] = f"{m[1]}# TODO: {m[2]}"
            continue


common.modify_files(f, common.args_python_files())
