import argparse
import os
import random
import re


def get_error_codes():
    here = os.path.dirname(os.path.realpath(__file__))
    codes = {}
    for s in open(os.path.join(here, "..", "src", "etc.h")).readlines():
        m = re.match(r"const int (\w+Error) = (\d+);", s)
        if m:
            codes[int(m[2])] = m[1]
    return codes


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
