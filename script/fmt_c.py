########################################
# sort case blocks


def case(i, dent):
    case_mark = dent + "(case .*|default):$"
    while 1:
        if not re.match(case_mark, lines[i]):
            raise ValueError(file + ":" + str(i + 1) + ": case not found")
        while re.match(case_mark, lines[i]):
            i += 1
        if dent + "{" == lines[i]:
            i += 1
            while dent + "}" != lines[i]:
                if re.match(case_mark, lines[i]):
                    raise ValueError(
                        file
                        + ":"
                        + str(i + 1)
                        + ": another case in the middle of block"
                    )
                i += 1
            i += 1
            return i
        else:
            while not re.match(case_mark, lines[i]) and dent + "}" != lines[i]:
                i += 1
            if not re.match(r"\s*\[\[fallthrough\]\];", lines[i - 1]):
                return i


def cases(i, dent):
    r = []
    while dent + "}" != lines[i]:
        j = case(i, dent)
        r.append(lines[i:j])
        i = j
    return i, r




def sort_cases(i, dent):
    j, cs = cases(i, dent)
    for c in cs:
        sort_case(c)
    cs = sorted(cs)
    lines[i:j] = flatten(cs)


def sort_case_blocks():
    for i in range(len(lines)):
        m = re.match(r"(\s*)switch \(.*\) {", lines[i])
        if m:
            sort_cases(i + 1, m[1])


########################################
# sort single-line elements


def var_key(x):
    m = re.match(r".* (\w+) = ", x)
    if m:
        x = m[1]
    else:
        m = re.match(r".* (\w+)\(", x)
        if m:
            x = m[1]
        else:
            m = re.match(r".* (\w+);", x)
            if m:
                x = m[1]
    return x.lower(), x


def sort_single():
    for i in range(len(lines)):
        if re.match(r"\s*// SORT$", lines[i]):
            if lines[i + 1].endswith("{"):
                continue
            j = i + 1
            # TODO: should this require the extra /?
            while not re.match(r"\s*///$", lines[j]):
                j += 1
            lines[i + 1 : j] = sorted(lines[i + 1 : j], key=var_key)


########################################
# sort multi-line elements


def get_multi_element(dent, i, j):
    i0 = i
    while re.match(r"\s*//", lines[i]):
        i += 1
    m = re.match(r"(\s*).*{$", lines[i])
    if not m:
        raise ValueError(file + ":" + str(i + 1) + ": inconsistent syntax")
    if m[1] != dent:
        raise ValueError(file + ":" + str(i + 1) + ": inconsistent indent")
    while lines[i] != dent + "}":
        i += 1
        if i > j:
            raise ValueError(file + ":" + str(i + 1) + ": inconsistent syntax")
    i += 1
    return lines[i0:i], i


def get_multi_elements(i, j):
    m = re.match(r"(\s*).*", lines[i])
    dent = m[1]
    xss = []
    while i < j:
        xs, i = get_multi_element(dent, i, j)
        xss.append(xs)
        while not lines[i]:
            i += 1
    return xss


def fn_key(xs):
    i = 0
    while re.match(r"\s*//", xs[i]):
        i += 1
    x = xs[i]
    m = re.match(r".* (\w+)\(", x)
    if m:
        x = m[1]
    return x.lower(), x, xs[i]


def sort_multi_block(i, j):
    xss = get_multi_elements(i, j)
    xss = sorted(xss, key=fn_key)
    for k in range(len(xss) - 1):
        xss[k].append("")
    xs = flatten(xss)
    lines[i:j] = xs


def sort_multi():
    i = 0
    while i < len(lines):
        if re.match(r"\s*// SORT$", lines[i]):
            i += 1
            if lines[i] == "":
                raise ValueError(
                    file + ":" + str(i + 1) + ": blank line after SORT directive"
                )
            if not lines[i].endswith("{") and not re.match(r"\s*//", lines[i]):
                continue
            j = i
            while not re.match(r"\s*///$", lines[j]):
                j += 1
            sort_multi_block(i, j)
        else:
            i += 1


########################################
# top level


def do():
    global lines
    lines = read_lines(file)
    old = lines[:]

    comments()
    sort_case_blocks()
    sort_single()
    sort_multi()

    if lines == old:
        return
    print(file)
    write_lines(file, lines)


for arg in args.files:
    if os.path.isdir(arg):
        for root, dirs, files in os.walk(arg):
            for file in files:
                ext = os.path.splitext(file)[1]
                if ext in (".cc", ".h"):
                    file = os.path.join(root, file)
                    do()
        continue
    file = arg
    do()
