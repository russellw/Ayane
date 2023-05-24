import os
import re


def end(i):
    for j in range(i, len(u)):
        if re.match(r"\s*#\s*", u[j]):
            return j
    raise Exception()


here = os.path.dirname(os.path.realpath(__file__))
for root, dirs, files in os.walk(arg):
    for file in files:
        ext = os.path.splitext(file)[1]
        if ext in (".cc", ".h"):
            file = os.path.join(root, file)

            u = open(os.path.join(src, file)).readlines()
            for i in range(len(u)):
                if re.match(r"\s*# SORT\s*", u[i]):
                    j = end(i)
                    v = u[i:j]
                    v.sort()
                    u[i:j] = v
            open(os.path.join(src, file), "w", newline="\n").writelines(u)
