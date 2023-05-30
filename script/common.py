import argparse
import os
import random
import re


# todo:sort
def get_error_codes():
    here = os.path.dirname(os.path.realpath(__file__))
    codes = {}
    for s in open(os.path.join(here, "..", "src", "etc.h")).readlines():
        m = re.match(r"const int (\w+Error) = (\d+);", s)
        if m:
            codes[int(m[2])] = m[1]
    return codes


def args_python_files():
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    files = args.files
    if not files:
        here = os.path.dirname(os.path.realpath(__file__))
        files = here, os.path.join(here, "..", "test")

    r = []
    for arg in files:
        if os.path.isdir(arg):
            for root, dirs, files in os.walk(arg):
                for file in files:
                    ext = os.path.splitext(file)[1]
                    if ext == ".py":
                        r.append(os.path.join(root, file))
            continue
        r.append(arg)
    return r


def modify_files(f, files):
    for file in files:
        v =list(s.rstrip() for s in  open(file).readlines())
        o = v.copy()

        w = f(v)
        if w:
            v = w

        if v != o:
            print(file)
            open(file, "w", newline="\n").writelines([s+'\n'for s in v])


def args_problems():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-n", "--number", help="max number of problems to attempt", type=int
    )
    parser.add_argument(
        "-r", "--random", help="attempt problems in random order", action="store_true"
    )
    parser.add_argument("-s", "--seed", help="random number seed", type=int)
    parser.add_argument(
        "-t", "--time", help="time limit per problem", type=float, default=60.0
    )
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    if args.seed is not None:
        args.random = 1
        random.seed(args.seed)

    return args


def args_files():
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="+")
    args = parser.parse_args()
    return args


def print_table(d):
    for a in d:
        print(a, end="\t")
        print(d[a])
