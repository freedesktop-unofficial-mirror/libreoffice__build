#!/usr/bin/env python

import sys, os, os.path, optparse

class ParseError(Exception): pass

def normalize_name (name):
    # Replace prohibited characters with someone sane.
    name = name.replace('-', '_')
    return name

def parse_build_lst (build_lst):

    # Read only the first line
    file = open(build_lst, 'r')
    while True:
        line = file.readline().strip()
        if line[0] != '#':
            break
    file.close()

    words = line.split()
    n = len(words)

    # Check line format to make sure it's formatted as expected.
    if n < 4:
        raise ParseError()
    if words[2] != ':' and words[2] != '::':
        raise ParseError()
    if words[-1] != 'NULL':
        raise ParseError()

    mod_name = normalize_name(words[1])
    depends = words[3:]
    for dep in depends:
        if dep == 'NULL':
            break

        names = dep.split(':')
        if len(names) > 2:
            raise ParseError()
        elif len(names) == 2:
            dep = names[1]

        dep = normalize_name(dep)
        print ("  " + mod_name + " -> " + dep + ";")

desc = """
Execute this script at the root directory of your OOo build tree.  It parses
all build.lst files found in the modules and outputs module dependency data 
in the dot compatible format."""

def main ():
    parser = optparse.OptionParser()
    parser.usage += "\n" + desc
    options, args = parser.parse_args()

    print ("digraph modules {")

    # Find all build.lst files.
    for mod in os.listdir(os.getcwd()):
        if not os.path.isdir(mod):
            # not a directory
            continue
        
        build_lst = mod + '/prj/build.lst'
        if not os.path.isfile(build_lst):
            # no build.lst found
            continue

        parse_build_lst(build_lst)
        
    print ("}")

if __name__ == '__main__':
    main()
