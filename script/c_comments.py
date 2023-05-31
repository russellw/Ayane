import re

import common


def special(s):
    m = re.match(r"(\s*)//\s*sort$", s, re.IGNORECASE)
    if m:
        return f"{m[1]}// SORT"

    m = re.match(r"(\s*)//\s*sortf$", s, re.IGNORECASE)
    if m:
        return f"{m[1]}// SORTF"

    m = re.match(r"(\s*)//\s*todo:\s*(.*)", s, re.IGNORECASE)
    if m:
        return f"{m[1]}// TODO: {m[2]}"

    if re.match(r"\s*//\s*https?:", s):
        return s


def words(s):
    v = s.split()
    return v


def lines(dent, v):
    width = 132 - len(dent) * 4 - 3
    s = ""
    r = []
    for t in v:
        if len(s) + 1 + len(t) > width:
            r.append(s)
            s = t
        else:
            if s:
                s += " "
            s += t
    assert s
    r.append(s)
    return [dent + "// " + s for s in r]


def f(v):
    i = 0
    while i < len(v):
        if not re.match(r"\s*//", v[i]):
            i += 1
            continue

        # special comment
        s = special(v[i])
        if s:
            v[i] = s
            continue

        # ordinary comments
        j = i
        w = []
        while j < len(v):
            m = re.match(r"(\s*)//(.*)", v[j])
            if not m:
                break
            if special(v[j]):
                break
            j += 1
            dent = m[1]
            w.append(m[2])
        r = lines(dent, words(" ".join(w)))
        v[i:j] = r
        i += len(r)


common.modify_files(f, common.args_c_files())
