import os
import random

tptp = os.getenv("TPTP")
if not tptp:
    raise Exception("TPTP environment variable not set")

v = []
for root, dirs, files in os.walk(tptp):
    for file in files:
        if "^" in file:
            continue
        ext = os.path.splitext(file)[1]
        if ext == ".p":
            file = os.path.join(root, file)
            v.append(file + "\n")
random.shuffle(v)
open("tptp.lst", "w", newline="\n").writelines(v)
