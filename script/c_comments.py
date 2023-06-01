import re

import common


# SORT
def capitalize(s):
    if len(s) == 1:
        return s
    if not s[1].isalpha():
        return s
    if "_" in s:
        return s
    for c in s:
        if c.isupper():
            return s
    return s.capitalize()


def f(v):
    # comments begin with exactly one space
    for i in range(len(v)):
        m = re.match(r"(\s*)//\s*(.*)", v[i])
        if m:
            v[i] = f"{m[1]}// {m[2]}"

    # other formatting
    i = 0
    while i < len(v):
        if not re.match(r"\s*//", v[i]):
            i += 1
            continue

        # special comment
        s = special(v[i])
        if s:
            v[i] = s
            i += 1
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


def sentence_end(s):
    if s.startswith("("):
        s = s[1:]
    if s.endswith(")"):
        s = s[:-1]

    if s in ("e.g.", "i.e."):
        return

    if s.endswith("."):
        return 1
    if s.endswith("?"):
        return 1


def special(s):
    m = re.match(r"(\s*)// sort$", s, re.IGNORECASE)
    if m:
        return f"{m[1]}// SORT"

    m = re.match(r"(\s*)// todo:\s*(.*)", s, re.IGNORECASE)
    if m:
        # TODO: change this so it doesn't show up in search
        return f"{m[1]}// TODO: {m[2]}"

    m = re.match(r"(\s*)// $", s)
    if m:
        return f"{m[1]}//"

    if re.match(r"\s*// https?:", s):
        return s
    if re.match(r"\s*// namespace", s):
        return s
    if re.match(r"\s*// clang-format off", s):
        return s
    if re.match(r"\s*// clang-format on", s):
        return s


def words(s):
    v = s.split()

    for i in range(len(v) - 1):
        if sentence_end(v[i]):
            v[i + 1] = capitalize(v[i + 1])

    i = len(v) - 1
    if v[i].endswith("."):
        v[i] = v[i][:-1]

    return v


common.modify_files(f, common.args_c_files())
