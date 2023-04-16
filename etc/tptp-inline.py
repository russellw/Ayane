import os
import sys
import pathlib


def inline(filename):
    lines = open(filename).readlines()
    for s in lines:
        if s.startswith("include("):
            tptp = os.getenv("TPTP")
            if not tptp:
                raise ValueError("TPTP environment variable not set")
            inline(tptp + "/" + s.split("'")[1])
            continue
        outf.write(s)


outf = open(sys.argv[2], "w")
inline(sys.argv[1])
