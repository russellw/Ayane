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


