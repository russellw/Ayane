import re

import common


def def1(v, i):
    dent = common.indent(v, i)

    # skip comments
    while (
        common.indent(v, i) == dent
        and not re.match(r"\s*#$", v[i])
        and re.match(r"\s*#", v[i])
    ):
        i += 1

    # there should be no more leading blank lines
    if i < len(v):
        assert v[i]

    # if there is no def, we are done
    if not (common.indent(v, i) == dent and re.match(r"\s*def \w+\(", v[i])):
        return

    # function body
    i += 1
    while common.indent(v, i) > dent:
        i += 1
    return i


def f(v):
    i = 0
    while i < len(v):
        if not re.match(r"\s*# SORT$", v[i]):
            i += 1
            continue
        i += 1

        # delete leading blank lines
        j = i
        while not v[j]:
            j += 1
        del v[i:j]
        j = i

        # sorting function definitions is different from sorting individual lines
        if def1(v, i):
            r = []
            while j < len(v):
                k = def1(v, j)
                if not k:
                    break
                r.append(trim(v[j:k]))
                j = k
            assert r
            r.sort(key=def_key)
            v[i:j] = common.cat(r)
        else:
            dent = common.indent(v, j)
            while common.indent(v, j) == dent and not re.match(r"\s*#", v[j]):
                j += 1
            assert common.indent(v, j) <= dent
            r = v[i:j]
            r.sort()
            v[i:j] = r

        i = j


def def_key(v):
    for s in v:
        if not re.match(r"\s*#", s):
            return s
    raise Exception(v)


def trim(v):
    i = len(v)
    while not v[i - 1]:
        i -= 1
    return v[:i]


common.modify_files(f, common.args_python_files())
