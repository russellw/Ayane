import argparse
import os
import re
import subprocess

parser = argparse.ArgumentParser(description="Run prover and verify proof")
parser.add_argument("files", nargs="+")
args = parser.parse_args()

subprocess.check_call("make")


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


formulas = {}


class Formula:
    def __init__(self, name, term, fm=None):
        self.name = name
        self.term = term
        self.fm = fm
        formulas[name] = self

    def __repr__(self):
        return self.name


def vars(s):
    i = 0
    r = set()
    while i < len(s):
        c = s[i]

        # variable
        if c.isupper():
            j = i
            while s[i].isalnum() or s[i] == "_":
                i += 1
            r.add(s[j:i])
            continue

        # word
        if c.isalpha():
            i += 1
            while i < len(s) and (s[i].isalnum() or s[i] == "_"):
                i += 1
            continue

        # quote
        if c in ("'", '"'):
            i += 1
            while s[i] != c:
                if s[i] == "\\":
                    i += 1
                i += 1
            i += 1
            continue

        # etc
        i += 1
    return r


def quantify(s, f):
    r = vars(s)
    if not r:
        f.write(s)
        return
    f.write("![")
    f.write(",".join(r))
    f.write("]:(")
    f.write(s)
    f.write(")")


def verify(filename):
    cmd = ["./ayane", "-t", "60", filename]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    out, err = p.communicate()
    out = str(out, "utf-8")
    for s in out.splitlines():
        m = re.match(r"cnf\((\w+), plain, (.+), introduced\(definition\)\)\.$", s)
        if m:
            Formula(m[1], m[2])
            continue

        m = re.match(
            r"cnf\((\w+), plain, (.+), inference\((cnf),\[status\(esa\)\],\[(\w+)\]\)\)\.",
            s,
        )
        if m:
            Formula(m[1], m[2])
            continue

        m = re.match(
            r"cnf\((\w+), plain, (.+), inference\(\w+,\[status\(thm\)\],\[(\w+)(,\w+)?\]\)\)\.",
            s,
        )
        if m:
            fm = [m[3]]
            if m[4]:
                fm.append(m[4][1:])
            Formula(m[1], m[2], fm)

    for c in formulas.values():
        if c.fm is not None:
            c.fm = [formulas[name] for name in c.fm]

    for c in formulas.values():
        if c.fm is not None:
            f = open("tmp.p", "w")
            for d in c.fm:
                f.write("tff(")
                f.write(d.name)
                f.write(",plain,")
                quantify(d.term, f)
                f.write(").\n")
            f.write("tff(")
            f.write(c.name)
            f.write(",plain,~(")
            quantify(c.term, f)
            f.write(")).\n")
            f.close()

            cmd = "bin/eprover", "--auto", "tmp.p"
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
            out, err = p.communicate()
            out = str(out, "utf-8")
            if p.returncode:
                print(out, end="")
                raise ValueError(str(p.returncode))
            if "Proof found" not in out:
                raise ValueError(out)


for arg in args.files:
    if not os.path.isfile(arg):
        for root, dirs, files in os.walk(arg):
            for filename in files:
                ext = os.path.splitext(filename)[1]
                if ext != ".p":
                    continue
                verify(os.path.join(root, filename))
        continue
    ext = os.path.splitext(arg)[1]
    if ext == ".lst":
        for filename in read_lines(arg):
            verify(filename)
        continue
    verify(arg)
