import re

import common


# SORT
def block(v, dent, i):
    # skip blank lines
    while i < len(v) and not v[i]:
        i += 1

    # end
    if indent(v, i) < dent or re.match(r"\s*//$", v[i]):
        return

    # skip comments
    while not re.match(r"\s*//$", v[i]) and re.match(r"\s*//", v[i]):
        i += 1

    # there should be no more leading blank lines
    if i < len(v):
        assert v[i]

    # braced block, probably a function definition
    if v[i - 1].endswith("{"):
        while not (indent(v, i) == dent and re.match(r"\s*}$", v[i])):
            i += 1

    return i + 1


def cat(v):
    r = []
    for w in v:
        r.extend(w)
    return r


def f(v):
    i = 0
    while i < len(v):
        if not re.match(r"\s*// SORT$", v[i]):
            i += 1
            continue

        dent = indent(v, i)
        i += 1

        j = i
        r = []
        while not re.match(r"\s*//", v[j]):
            k = block(v, dent, j)
            r.append(v[j:k])
            j = k
        assert indent(v, i) == dent
        assert r
        r.sort()
        v[i:j] = cat(r)

        i = j + 1


def indent(v, i):
    if i == len(v):
        return -1
    s = v[i]
    if not s:
        return 1000000
    j = 0
    while s[j] == "\t":
        j += 1
    return j


common.modify_files(f, common.args_c_files())
