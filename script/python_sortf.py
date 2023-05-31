import re

import common


# SORTF
def cat(v ):
    r = []
    for w in v:
        r.extend(w)
    return r


def def1(v, i):
    dent = indent(v, i)
    while indent(v, i) == dent and re.match(r"\s*#", v[i]):
        i += 1
    if not (indent(v, i) == dent and re.match(r"\s*def ", v[i])):
        return
    i += 1
    while indent(v, i) > dent:
        i += 1
    return i


def def_name(v):
    for s in v:
        m = re.match(r"\s*def (\w+)", s)
        if m:
            return m[1]
    raise Exception(v)


def f(v):
    i = 0
    while i < len(v):
        if not re.match(r"\s*# SORTF\s*$", v[i]):
            i += 1
            continue
        i += 1
        j = i
        while not v[j]:
            j += 1
        r = []
        while j < len(v):
            k = def1(v, j)
            if not k:
                break
            r.append(trim(v[j:k]))
            j = k
        if not r:
            raise Exception(i)
        r.sort(key=def_name)
        v[i:j] = cat(r)


def indent(v, i):
    if i == len(v):
        return -1
    s = v[i]
    if not s:
        return 1000000
    j = 0
    while s[j] == " ":
        j += 1
    if s[j] == "\t":
        raise Exception("file indented with tabs")
    return j


def trim(v):
    i = len(v)
    while not v[i - 1]:
        i -= 1
    return v[:i]


common.modify_files(f, common.args_python_files())
