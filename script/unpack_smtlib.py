import os

import common
import smtlib

args = common.args_files()
os.chdir("C:/smtlib")


def want(file):
    ext = os.path.splitext(file)[1]
    if ext != ".zip":
        return

    file = file.upper()
    for s in smtlib.unsupported_logics:
        if file.startswith(s):
            return

    return 1


for arg in args.files:
    for root, dirs, files in os.walk(arg):
        for file in files:
            if want(file):
                file = os.path.join(root, file)
                print(file)
