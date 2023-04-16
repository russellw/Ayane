#!/usr/bin/python3
import inspect
import subprocess
import re
import sys
import logging

logger = logging.getLogger()
logger.addHandler(logging.StreamHandler(sys.stdout))
logger.setLevel(logging.DEBUG)

# numbers larger than 2000 silently fail
sys.setrecursionlimit(2000)

subprocess.check_call(["make", "debug"])


def dbg(a):
    info = inspect.getframeinfo(inspect.currentframe().f_back)
    logger.debug(f"{info.filename}:{info.function}:{info.lineno}: {repr(a)}")


def remove(s, i):
    s = list(s)
    del s[i]
    return s


def check_tuples(x):
    if isinstance(x, tuple):
        for y in x:
            check_tuples(y)
        return
    if isinstance(x, list):
        raise ValueError(x)


def imp(x, y):
    return "|", ("~", x), y


def size(x):
    if type(x) in (list, tuple):
        n = 0
        for y in x:
            n += size(y)
        return n
    return 1


######################################## parser


def read_tptp(filename, xs, select=True):
    text = open(filename).read()
    if text and text[-1] != "\n":
        text += "\n"

    # tokenizer
    ti = 0
    tok = ""

    def err(msg):
        line = 1
        for i in range(ti):
            if text[i] == "\n":
                line += 1
        raise ValueError(f"{filename}:{line}: {repr(tok)}: {msg}")

    def lex():
        nonlocal ti
        nonlocal tok
        while ti < len(text):
            c = text[ti]

            # space
            if c.isspace():
                ti += 1
                continue

            # line comment
            if c in ("%", "#"):
                i = ti
                while text[ti] != "\n":
                    ti += 1
                continue

            # block comment
            if text[ti : ti + 2] == "/*":
                ti += 2
                while text[ti : ti + 2] != "*/":
                    ti += 1
                ti += 2
                continue

            # word
            if c.isalpha() or c == "$":
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

            # number
            if c.isdigit() or (c == "-" and text[ti + 1].isdigit()):
                # integer part
                i = ti
                ti += 1
                while text[ti].isalnum():
                    ti += 1

                # rational
                if text[ti] == "/":
                    ti += 1
                    while text[ti].isdigit():
                        ti += 1

                # real
                else:
                    if text[ti] == ".":
                        ti += 1
                        while text[ti].isalnum():
                            ti += 1
                    if text[ti - 1] in ("e", "E") and text[ti] in ("+", "-"):
                        ti += 1
                        while text[ti].isdigit():
                            ti += 1

                tok = text[i:ti]
                return

            # punctuation
            if text[ti : ti + 3] in ("<=>", "<~>"):
                tok = text[ti : ti + 3]
                ti += 3
                return
            if text[ti : ti + 2] in ("!=", "=>", "<=", "~&", "~|"):
                tok = text[ti : ti + 2]
                ti += 2
                return
            tok = c
            ti += 1
            return

        # end of file
        tok = None

    def eat(o):
        if tok == o:
            lex()
            return True

    def expect(o):
        if tok != o:
            err(f"expected '{o}'")
        lex()

    # terms
    def args():
        expect("(")
        r = []
        if tok != ")":
            r.append(atomic_term())
            while tok == ",":
                lex()
                r.append(atomic_term())
        expect(")")
        return tuple(r)

    def atomic_term():
        o = tok

        # higher-order terms
        if tok == "!":
            raise "Inappropriate"

        # syntax sugar
        if eat("$greater"):
            s = args()
            return "$less", s[1], s[0]
        if eat("$greatereq"):
            s = args()
            return "$lesseq", s[1], s[0]

        lex()
        if tok == "(":
            s = args()
            return (o,) + s

        return o

    def infix_unary():
        x = atomic_term()
        o = tok
        if o == "=":
            lex()
            return "=", x, atomic_term()
        if o == "!=":
            lex()
            return "~", ("=", x, atomic_term())
        return x

    def unitary_formula():
        o = tok
        if o == "(":
            lex()
            x = logic_formula()
            expect(")")
            return x
        if o == "~":
            lex()
            return "~", unitary_formula()
        if o in ("!", "?"):
            lex()

            # variables
            expect("[")
            v = []
            v.append(atomic_term())
            while tok == ",":
                lex()
                v.append(atomic_term())
            expect("]")

            # body
            expect(":")
            x = o, tuple(v), unitary_formula()
            return x
        return infix_unary()

    def logic_formula():
        x = unitary_formula()
        o = tok
        if o in ("&", "|", "<=>"):
            lex()
            return o, x, unitary_formula()
        if o == "=>":
            lex()
            return imp(x, unitary_formula())
        if o == "<=":
            lex()
            return imp(unitary_formula(), x)
        if o == "<~>":
            lex()
            return "~", ("<=>", x, unitary_formula())
        if o == "~&":
            lex()
            return "~", ("&", x, unitary_formula())
        if o == "~|":
            lex()
            return "~", ("|", x, unitary_formula())
        return x

    # top level
    def ignore():
        if eat("("):
            while not eat(")"):
                ignore()
            return
        lex()

    def selecting(name):
        return select is True or name in select

    def annotated_formula():
        lex()
        expect("(")

        # name
        name = atomic_term()
        expect(",")

        # role
        role = atomic_term()
        expect(",")

        if role == "type":
            while tok != ")":
                ignore()
        else:
            x = logic_formula()
            if selecting(name):
                if role == "conjecture":
                    x = "~", x
                xs.append(x)

        # annotations
        if tok == ",":
            while tok != ")":
                ignore()

        # end
        expect(")")
        expect(".")

    def include():
        lex()
        expect("(")

        # tptp
        tptp = os.getenv("TPTP")
        if not tptp:
            err("TPTP environment variable not set")

        # file
        filename1 = atomic_term()

        # select
        select1 = select
        if eat(","):
            expect("[")
            select1 = []
            while True:
                name = atomic_term()
                if selecting(name):
                    select1.append(name)
                if not eat(","):
                    break
            expect("]")

        # include
        read_tptp(tptp + "/" + filename1, xs, select1)

        # end
        expect(")")
        expect(".")

    lex()
    header = False
    while tok:
        if tok in ("cnf", "fof", "tff"):
            annotated_formula()
            continue
        if tok == "include":
            include()
            continue
        err("unknown language")


######################################## printing

outf = None


def pr(x):
    if x is not str:
        x = str(x)
    outf.write(x)


def prargs(x):
    pr("(")
    for i in range(1, len(x)):
        if i > 1:
            pr(",")
        prterm(x[i])
    pr(")")


def need_parens(x, parent):
    if not parent:
        return
    if x[0] in ("&", "<=>", "|"):
        return parent[0] in ("&", "<=>", "?", "!", "~", "|")


def prterm(x, parent=None):
    if isinstance(x, tuple):
        o = x[0]
        # infix
        if o == "=":
            prterm(x[1])
            pr("=")
            prterm(x[2])
            return
        if o in ("&", "<=>", "|"):
            if need_parens(x, parent):
                pr("(")
            assert len(x) == 3
            for i in range(1, len(x)):
                if i > 1:
                    pr(f" {o} ")
                prterm(x[i], x)
            if need_parens(x, parent):
                pr(")")
            return

        # prefix/infix
        if o == "~":
            pr("~")
            prterm(x[1], x)
            return

        # prefix
        if o in ("?", "!"):
            pr(o)
            pr("[")
            v = x[1]
            for i in range(len(v)):
                if i:
                    pr(",")
                y = v[i]
                pr(y)
            pr("]:")
            prterm(x[2], x)
            return
        pr(o)
        prargs(x)
        return
    pr(x)


formnames = 0


def prformula(x):
    global formnames
    formnames += 1
    pr("tff")
    pr("(")

    # name
    pr(formnames)
    pr(", ")

    # role
    pr("plain")
    pr(", ")

    # content
    prterm(x)

    # end
    pr(").\n")


def write_tmp(xs):
    global formnames
    global outf
    formnames = 0
    outf = open("tmp.p", "w")
    for x in xs:
        prformula(x)
    outf.close()


######################################## shrink


def shrink(x):
    if type(x) is not tuple:
        return [x]
    o = x[0]
    if o in ("!", "?"):
        r = ["$true", "$false"]
        xs1 = shrink(x[2])
        for y in xs1:
            r.append((o, x[1], y))
        return r
    if o in ("&", "|", "<=>"):
        r = ["$true", "$false"]
        xs1 = shrink(x[1])
        xs2 = shrink(x[2])
        r.extend(xs1)
        r.extend(xs2)
        for y in xs1:
            r.append((o, y, x[2]))
        for y in xs2:
            r.append((o, x[1], y))
        return r
    if o in ("~",):
        r = ["$true", "$false"]
        xs1 = shrink(x[1])
        r.extend(xs1)
        for y in xs1:
            r.append((o, y))
        return r
    return [x]


def shrinks(xs):
    r = []
    for i in range(len(xs)):
        for x in shrink(xs[i]):
            ys = xs[:i] + [x] + xs[i + 1 :]
            r.append(ys)
    return r


######################################## top level


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


def good_test(xs):
    write_tmp(xs)
    r_eprover = run_eprover("tmp.p")
    if not solved(r_eprover):
        return

    write_tmp(xs)
    r_ayane = run_ayane("tmp.p")
    if not solved(r_ayane):
        return

    return r_eprover != r_ayane


def do_file(filename):
    xs = []
    read_tptp(filename, xs)
    assert good_test(xs)
    while 1:
        print(xs)
        print(f"size: {size(xs)}")
        xss = shrinks(xs)
        for ys in xss:
            print(ys)
            if size(ys) >= size(xs):
                continue
            if not good_test(ys):
                continue
            xs = ys
            break
        else:
            write_tmp(xs)
            exit(0)


do_file(sys.argv[1])
