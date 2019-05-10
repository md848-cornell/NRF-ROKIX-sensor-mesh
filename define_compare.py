
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
            if key not in both.keys():
                both[key] = [v1[key], v2[key]]
    for key in v1:
        if key not in both.keys():
            if key not in both.keys():
                only1[key] = [v1[key]]
    for key in v2:
        if key not in both.keys():
            if key not in both.keys():
                only2[key] = [v2[key]]
    return both, only1, only2


def filt(d, kwl):
    nd = {}
    for key in d:
        found = False
        for kw in kwl:
            if kw in key:
                found = True
                continue
        if found:
            nd[key] = d[key]
    return nd


def compare_defs(f1,f2):
    t1 = readfile(f1)
    t2 = readfile(f2)
    
    p = '#define'
    l1 = get_lines(t1,p)
    l2 = get_lines(t2,p)

    l1, l2 = remove_in_both(l1,l2)

    v1 = get_vals(l1)
    v2 = get_vals(l2)

    b, k1, k2 = organize(v1,v2)

    #b = filt(b, ['UART','LOG'])
    #k1 = filt(k1, ['UART','LOG'])
    #k2 = filt(k2, ['UART','LOG'])

    print('### ',p,' THAT DIFFER')
    for key in b:
        print(key, ':', b[key])
        #print('#define', key, b[key][1])
    print('### ',p,' IN FILE:', f1)
    for key in k1:
        print(key, ':', k1[key])
        #print('#define', key, k1[key][0])
    print('### ',p,' IN FILE: ', f2)
    for key in k2:
        print(key, ':', k2[key])
        #print('#define', key, k2[key][0])

    if False:
        print('### DEFINES ',p,' IN FILE: ', f2)
        for key in k2:
            print('#ifndef '+key)
            print('#define '+key+' '+k2[key][0])
            print('#endif')


def main():
    if len(sys.argv) < 3:
        print('no files specified')
        return
    f1 = sys.argv[1]
    f2 = sys.argv[2]
    compare_defs(f1,f2)


if __name__ == '__main__':
    main()
