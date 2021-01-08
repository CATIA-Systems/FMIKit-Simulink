import os
import re


root = os.path.abspath(__file__)
root = os.path.dirname(root)

internal_include = re.compile(r'#include "(FMI.*|fmi.*)".*')

between_quotes = re.compile(r'"(.*?)"')

included = set()


def open_file(filename):
    for folder in ['include', 'src']:
        path = os.path.join(root, folder, filename)
        if os.path.isfile(path):
            return open(path, 'r')


def expand_includes(out, filename):
    with open_file(filename) as file:
        for line in file.readlines():
            if internal_include.match(line):
                inc_file = between_quotes.findall(line)[0]
                if inc_file not in included:
                    expand_includes(out, inc_file)
                    included.add(filename)
            else:
                out.write(line)


with open(os.path.join(root, 'sfun_fmurun.c'), 'w') as sfun:
    expand_includes(sfun, 'sfun_fmurun.c')
