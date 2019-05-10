
import os
import sys


def readfile(fn):
    f = open(fn, 'r')
    t = f.read()
    f.close()
    return t

def get_lines(t,p):
    out = []
    lines = t.split('\n')
    for line in lines:
        if line.startswith(p):
            out += [line]
    return out

def remove_in_both(l1,l2):
    new_l1 = []
    for line in l1:
        if line not in l2:
            new_l1 += [line]
    new_l2 = []
    for line in l2:
        if line not in l1:
            new_l2 += [line]
    return new_l1,new_l2


def get_vals(lines):
    v = {}
    for line in lines:
        l = line.split(' ')
        var = l[1]
        val = ' '.join(l[2:])
        v[var] = val
    return v

def organize(v1,v2):
    both = {}
    only1 = {}
    only2 = {}
    for key in v1:
        if key in v2.keys():
            both[key] = [v1[key], v2[key]]
    for key in v1:
        if key not in both.keys():
            only1[key] = [v1[key]]
    for key in v2:
        if key not in both.keys():
            only2[key] = [v2[key]]
    return both, only1, only2


def compare_defs(f1):
    t1 = readfile(f1)
    
    p = '#define'
    l1 = get_lines(t1,p)
    for line in l1:
        print(line)


def main():
    if len(sys.argv) < 2:
        print('no files specified')
        return
    f1 = sys.argv[1]
    compare_defs(f1)


if __name__ == '__main__':
    main()
