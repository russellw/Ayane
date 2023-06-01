import re

import common


# SORT
def block(v, dent, i):
    assert indent(v, i) == dent
    assert re.match(r"\s*(case .*|default):", v[i])

    while re.match(r"\s*(case .*|default):", v[i]):
        i += 1

    # braced block
    if v[i - 1].endswith("{"):
        while not (indent(v, i) == dent and re.match(r"\s*}$", v[i])):
            i += 1
        return i + 1

    # blocks may be chained with [[fallthrough]]
    while 1:
        while indent(v, i) != dent:
            i += 1
        assert re.match(r"\s*(case .*|default):", v[i]) or re.match(r"\s*}", v[i])
        if not re.match(r"\s*\[\[fallthrough\]\];", v[i - 1]):
            return i
        while re.match(r"\s*(case .*|default):", v[i]):
            i += 1


def cat(v):
    r = []
    for w in v:
        r.extend(w)
    return r


def f(v):
    i = 0
    while i < len(v):
        if not re.match(r"(\s*)switch \(.*\) {", v[i]):
            i += 1
            continue

        dent = indent(v, i)
        i += 1

        j = i
        r = []
        while not re.match(r"\s*}", v[i]):
            k = block(v, dent, j)
            r.append(v[j:k])
            j = k
        assert indent(v, i) == dent
        assert r
        r.sort()
        v[i:j] = cat(r)

        i = j + 1


def indent(v, i):
    s = v[i]
    if not s:
        return 1000000
    j = 0
    while s[j] == "\t":
        j += 1
    return j


def iscase(v, i):
    if re.match(r"\s*case .*:", v[i]):
        return 1
    if re.match(r"\s*default:", v[i]):
        return 1


common.modify_files(f, common.args_c_files())
