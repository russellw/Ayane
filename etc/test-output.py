import os
import re
import subprocess

efiles = []


def write_lines(filename, lines):
    with open(filename, "w") as f:
        for s in lines:
            f.write(s + "\n")


def act(f):
    p = subprocess.Popen(
        ["./ayane.exe", "-t10", f],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = p.communicate()
    stdout = str(stdout, "utf-8")
    stderr = str(stderr, "utf-8")
    if stderr:
        print(f)
        print(stderr, end="")
        exit(1)
    m = re.search(r"^\s*(\d+)\s+subsumes timeout$", stdout)
    if not m:
        return
    print(f + ":" + m[1])
    efiles.append(f)


for root, dirs, files in os.walk("/TPTP/Problems"):
    for f in files:
        act(root + "/" + f)
write_lines("test_output.lst", efiles)
