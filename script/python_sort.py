import re

import common

def cat(v):
    r = []
    for w in v:
        r.extend(w)
    return r


def block(v, i):
    dent = indent(v, i)

    #already at the end
    if not (indent(v,i)==dent and not  re.match(r"\s*#$", v[i])):
        return

    #skip comments
    while indent(v,i)==dent and not  re.match(r"\s*#$", v[i])  and re.match(r"\s*#", v[i]):
        i += 1

    #a comment in a sort block should be followed by something to sort
    if not (indent(v,i)==dent and not  re.match(r"\s*#$", v[i])):
        raise Exception(i)

    #def
    if   re.match(r"\s*def \w+\(", v[i]):
        i += 1
        while indent(v, i) > dent:
            i += 1
        return i

    #assume just one line
    return i+1




def f(v):
    i = 0
    while i < len(v):
        if not re.match(r"\s*# SORT\s*$", v[i]):
            i += 1
            continue
        i += 1
        j = i
        while not v[j]:
            j += 1
        r = []
        while j < len(v):
            k = block(v, j)
            if not k:
                break
            r.append(trim(v[j:k]))
            j = k
        if not r:
            raise Exception(i)
        r.sort(key=key)
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
def key(v):
    for s in v:
        if re.match(r"\s*#", s):
            continue
        if   re.match(r"\s*def \w+\(", s):
            return' '+s
        return s
    raise Exception(v)



def trim(v):
    i = len(v)
    while not v[i - 1]:
        i -= 1
    return v[:i]





common.modify_files(f, common.args_python_files())
