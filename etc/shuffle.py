import random
import sys

fname = sys.argv[1]
with open(fname) as f:
    content = f.readlines()
random.shuffle(content)
for s in content:
    print(s.strip())
