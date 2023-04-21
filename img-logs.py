import argparse
import os
import re
import sys

from PIL import Image

# numbers above 2000 silently fail
sys.setrecursionlimit(2000)

parser = argparse.ArgumentParser()
parser.add_argument("files", nargs="+")
args = parser.parse_args()


def clause(text):
    ti = 0
    tok = ""

    def lex():
        nonlocal ti
        nonlocal tok
        while ti < len(text):
            c = text[ti]

            # space
            if c.isspace():
                ti += 1
                continue

            # word
            if c.isalnum() or c == "$" or c == "-" and text[ti + 1].isdigit():
                i = ti
                ti += 1
                while text[ti].isalnum() or text[ti] == "_":
                    ti += 1
                tok = text[i:ti]
                return

            # quote
            if c in ("'", '"'):
                i = ti
                ti += 1
                while text[ti] != c:
                    if text[ti] == "\\":
                        ti += 1
                    ti += 1
                ti += 1
                tok = text[i:ti]
                return

            # punctuation
            if text[ti : ti + 2] == "!=":
                tok = text[ti : ti + 2]
                ti += 2
                return
            tok = c
            ti += 1
            return

        # end of input
        tok = None

    def eat(o):
        if tok == o:
            lex()
            return True

    def expect(o):
        if not eat(o):
            raise Exception(tok)

    def atomic_term():
        a = tok
        lex()
        if eat("("):
            a = [a]
            if tok != ")":
                a.append(atomic_term())
                while eat(","):
                    a.append(atomic_term())
            expect(")")
            return tuple(a)
        return a

    def infix_unary():
        a = atomic_term()
        if eat("="):
            return "=", a, atomic_term()
        if eat("!="):
            return "not", ("=", a, atomic_term())
        return a

    def unitary_formula():
        if eat("~"):
            return "not", unitary_formula()
        return infix_unary()

    lex()
    expect("cnf")
    expect("(")

    # name
    lex()
    expect(",")

    # role
    lex()
    expect(",")

    # literals
    expect("(")
    neg = []
    pos = []
    while True:
        a = unitary_formula()
        if a[0] == "not":
            a = a[1]
            assert a[0] != "not"
            neg.append(a)
        else:
            pos.append(a)
        if not eat("|"):
            return neg, pos


squares = set()


def put(x, y, a):
    if isinstance(a, str):
        squares.add((x, y, a))
        return x + 1, y + 1

    assert isinstance(a[0], str)
    squares.add((x, y, a[0]))
    y += 1

    y2 = y
    for b in a[1:]:
        x, y1 = put(x, y, b)
        y2 = max(y2, y1)
    return x, y2


def hash_rgb(a):
    n = hash(a) & 0xFFFFFF
    b = n.to_bytes(3, byteorder="little")
    return b[0], b[1], b[2]


def do(file):
    squares.clear()
    x2 = 0
    y = 0
    for s in open(file):
        if s.startswith("#cnf") and s.endswith("\n"):
            s = s[1:]
            neg, pos = clause(s)
            x = 0
            y2 = y
            for a in neg:
                x, y1 = put(x, y, a)
                y2 = max(y2, y1)
            x += 1
            for a in pos:
                x, y1 = put(x, y, a)
                y2 = max(y2, y1)
            x2 = max(x2, x)
            y = y2

    pixels = [(0, 0, 0)] * (x2 * y2)
    for x, y, s in squares:
        pixels[y * x2 + x] = hash_rgb(s)

    image = Image.new("RGB", (x2, y2))
    image.putdata(pixels)
    image.save("a.png")


for arg in args.files:
    if os.path.isdir(arg):
        for root, dirs, files in os.walk(arg):
            for file in files:
                ext = os.path.splitext(file)[1]
                if ext == ".log":
                    do(os.path.join(root, file))
        continue
    ext = os.path.splitext(arg)[1]
    if ext == ".lst":
        for file in open(arg):
            do(file.strip())
        continue
    do(arg)
