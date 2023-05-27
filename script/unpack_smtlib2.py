import os
import subprocess

import smtlib

import common

args = common.args_files()
os.chdir("C:/smtlib/2")


def want(file):
    ext = os.path.splitext(file)[1]
    if ext != ".zip":
        return
    return 1


for arg in args.files:
    for root, dirs, files in os.walk(arg):
        for file in files:
            if want(file):
                file = os.path.join(root, file)
                print(file)
                cmd = "7z", "x", file
                subprocess.call(cmd)
