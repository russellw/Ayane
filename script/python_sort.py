import os
import re


def end(i):
    for j in range(i, len(u)):
        if re.match(r"\s*#\s*$", u[j]):
            return j
    raise Exception()


here = os.path.dirname(os.path.realpath(__file__))
for root, dirs, files in os.walk(here):
    for file in files:
        ext = os.path.splitext(file)[1]
        if ext == ".py":
            file = os.path.join(root, file)

            u = open(file).readlines()
            old = u.copy()

            for i in range(len(u)):
                if re.match(r"\s*# SORT\s*$", u[i]):
                    j = end(i + 1)
                    v = u[i + 1 : j]
                    v.sort()
                    u[i + 1 : j] = v

            if u != old:
                print(file)
                open(file, "w", newline="\n").writelines(u)
