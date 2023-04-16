#!/usr/bin/python3
import inspect
import subprocess
import re
import sys
import logging

subprocess.check_call(["make", "debug"])


def solved(r):
    if r == "Unsatisfiable":
        return 1
    if r == "Satisfiable":
        return 1


def run_eprover(filename):
    cmd = ["bin/eprover", "--generated-limit=10000", filename]
    p = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = p.communicate()
    stdout = str(stdout, "utf-8")
    stderr = str(stderr, "utf-8")
    if stderr:
        print(stderr, end="")
        exit(1)
    if p.returncode not in (0, 1, 8):
        raise Exception(str(p.returncode))
    if "Proof found" in stdout:
        return "Unsatisfiable"
    if "No proof found" in stdout:
        return "Satisfiable"


def run_ayane(filename):
    cmd = ["./ayane", "-t3", filename]
    p = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = p.communicate()
    stdout = str(stdout, "utf-8")
    stderr = str(stderr, "utf-8")
    if stderr:
        print(stderr, end="")
        exit(1)
    if p.returncode:
        raise Exception(str(p.returncode))
    m = re.search(r"SZS status (\w+) for \w+", stdout)
    if not m:
        raise Exception(stdout)
    return m[1]


def good_test(filename):
    r_eprover = run_eprover(filename)
    if not solved(r_eprover):
        return

    r_ayane = run_ayane(filename)
    if not solved(r_ayane):
        return

    return r_eprover != r_ayane


print(good_test(sys.argv[1]))
