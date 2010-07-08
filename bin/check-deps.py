#!/usr/bin/env python

import sys, os, os.path, optparse

class ParseError(Exception): pass

class SingleModeError(Exception): pass

arg_desc = "module1 module2 ..."

desc = """
Execute this script at the root directory of your OOo build tree.  It parses
all build.lst files found in the modules and outputs module dependency data 
in the dot compatible format.

When no arguments are given, it prints dependencies of all discovered 
modules.  When module names are given as arguments, it only traces 
dependencies of those modules."""

class Module(object):

    def __init__ (self, name):
        self.name = name
        self.deps  = {} # dependents
        self.precs = {} # precedents

class DepsCheker(object):

    def __init__ (self):
        self.modules = {}
        self.selected = []

    def __normalize_name (self, name):
        # Replace prohibited characters with someone sane.
        name = name.replace('-', '_')
        return name
    
    def __insert_depend (self, mod, dep):

        # precedent to dependent
        if not self.modules.has_key(mod):
            self.modules[mod] = Module(mod)
        obj = self.modules[mod]
        obj.deps[dep] = True

        # dependent to precedent
        if not self.modules.has_key(dep):
            self.modules[dep] = Module(dep)
        obj = self.modules[dep]
        obj.precs[mod] = True

    def __parse_build_lst (self, build_lst):
    
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
    
        mod_name = self.__normalize_name(words[1])
        depends = words[3:]
        for dep in depends:
            if dep == 'NULL':
                break
    
            names = dep.split(':')
            if len(names) > 2:
                raise ParseError()
            elif len(names) == 2:
                dep = names[1]
    
            dep = self.__normalize_name(dep)
            self.__insert_depend(mod_name, dep)

    def run (self, selected):
    
        # modules we want to print dependency on.
        self.selected = selected

        # Find all build.lst files.
        for mod in os.listdir(os.getcwd()):
            if not os.path.isdir(mod):
                # not a directory
                continue
            
            build_lst = mod + '/prj/build.lst'
            if not os.path.isfile(build_lst):
                # no build.lst found
                continue
    
            self.__parse_build_lst(build_lst)
            
    def print_dot_all (self):
        print ("digraph modules {")

        if len(self.selected) == 0:
            mods = self.modules.keys()
            for mod in mods:
                deps = self.modules[mod].deps.keys()
                for dep in deps:
                    self.__print_dot_dep_line(mod, dep)
        else:
            # determine involved modules.
            self.__processed_mods = {}
            for selected in self.selected:
                if not self.modules.has_key(selected):
                    raise ParseError()
                if len(self.modules[selected].deps) > 0:
                    self.__trace_deps(self.modules[selected])
                else:
                    print ("    " + selected + ";")

        print ("}")

    def print_dot_single (self, mods):
        print ("digraph modules {")
        for mod in mods:

            if not self.modules.has_key(mod):
                continue

            obj = self.modules[mod]

            if len(obj.precs) == 0 and len(obj.deps) == 0:
                # No dependencies.  Just print the module.
                print ("    " + mod + ";")
                continue

            for prec in obj.precs.keys():
                self.__print_dot_dep_line(prec, obj.name)
            for dep in obj.deps.keys():
                self.__print_dot_dep_line(obj.name, dep)

        print ("}")

    def __trace_deps (self, obj):
        if self.__processed_mods.has_key(obj.name):
            return
        self.__processed_mods[obj.name] = True

        for dep_name in obj.deps.keys():
            if not self.modules.has_key(dep_name):
                raise ParseError()
            self.__print_dot_dep_line(obj.name, dep_name)
            self.__trace_deps(self.modules[dep_name])

    def __print_dot_dep_line (self, prec, dep):
        print ("    " + prec + " -> " + dep + ";")


if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.usage += " " + arg_desc + "\n" + desc
    parser.add_option("-s", "--single", action="store_true", dest="single", default=False,
        help="Print only immediate dependencies of specified modules.")
    options, args = parser.parse_args()

    checker = DepsCheker()
    if options.single:
        if len(args) == 0:
            # single mode requires module names.
            raise SingleModeError()
        checker.run(args)
        checker.print_dot_single(args)

    else:
        checker.run(args)
        checker.print_dot_all()
