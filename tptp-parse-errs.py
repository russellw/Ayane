import os
import re
import subprocess

efiles = []
etext = []


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def write_lines(filename, lines):
    with open(filename, "w") as f:
        for s in lines:
            f.write(s + "\n")


def emit(f0, e):
    print(f0)
    print(e, end="")
    f = "C:" + e.split(":")[1]
    efiles.append(f)
    line = int(e.split(":")[2])
    i = line - 1
    xs = read_lines(f)
    j = min(i + 2, len(xs))
    if i:
        i -= 1
    xs = xs[i:j]
    for x in xs:
        print(x)
    etext.append(f0)
    etext.append(e.rstrip())
    etext.extend(xs)


def act(f):
    if "-" in f:
        return
    print(f)
    p = subprocess.Popen(
        ["./ayane.exe", "-cnf", f],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = p.communicate()
    stdout = str(stdout, "utf-8")
    stderr = str(stderr, "utf-8")
    if stderr:
        emit(f, stderr)


subprocess.check_call(r"C:\ayane\scripts\build-release.bat", shell=True)
for root, dirs, files in os.walk("C:\\TPTP\\Problems"):
    for f in files:
        act(root + "\\" + f)
write_lines("efiles.lst", efiles)
write_lines("etext.log", etext)
