import time

import common
import smtlib

args = common.args_problems()
problems = smtlib.get_problems(args)

d = {}
for file in problems:
    L = smtlib.get_logic(file)
    d[L] = d.get(L, 0) + 1
common.print_table(d)
print(len(d))
