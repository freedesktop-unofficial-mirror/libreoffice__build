#!/usr/bin/env python

import sys, os, os.path, optparse, subprocess

arg_desc = ""

desc = """
Run this script at the root of OOo source tree."""

def error (msg):
    sys.stderr.write(msg + "\n")

class ErrorBase(Exception):
    
    def __init__ (self, name, msg):
        self.value = "%s: %s"%(name, msg)

    def __str__ (self):
        return repr(self.value)

class ParseError(ErrorBase):
    
    def __init__ (self, msg):
        ErrorBase.__init__(self, "ParseError", msg)

# Parse each .scp file.
class Scp2Parser(object):

    class Type:
        File       = 0
        Directory  = 1
        FolderItem = 2

    NodeTypes = [
        'DataCarrier',
        'Directory', 
        'File', 
        'Folder',
        'FolderItem', 
        'Installation', 
        'Module',
        'Profile',
        'ProfileItem',
        'RegistryItem', 
        'ScpAction',
        'Shortcut',
        'StarRegistry',
        'Unixlink',
        'WindowsCustomAction'
    ]

    def __init__ (self, content):
        self.content = content

    def tokenize (self):
        self.tokens = []
        i = 0
        n = len(self.content)
        token = ''
        while i < n:
            c = self.content[i]
            if c in '\t\n;':
                c = ' '

            if c == ' ':
                if len(token) > 0:
                    self.tokens.append(token)
                    token = ''
            else:
                token += c
            i += 1

    def parse (self):
        if len(self.tokens) == 0:
            # No tokens to parse.  Bail out.
            return
            
        self.i = 0
        self.n = len(self.tokens)
        while self.i < self.n:
            t = self.token()
            if t in Scp2Parser.NodeTypes:
                name, attrs = self.__parseEntity()
                print name
                print attrs
            else:
                raise ParseError("Unknown block type: %s"%t)

            self.next()

    def next (self):
        self.i += 1

    def token (self):
        return self.tokens[self.i]

    def __parseEntity (self):
        self.next()
        name = self.token()
        left = True
        attr_name = ''
        attr_value = ''
        attrs = {}
        self.next()
        while self.token() != 'End':
            if self.token() == '=':
                if not left:
                    raise ParseError("multiple '='s in a single line")

                if len(attr_name) == 0:
                    raise ParseError("empty attribute name")

                left = False
            
            elif left:
                attr_name = self.token()
            else:
                attr_value = self.token()
                attrs[attr_name] = attr_value
                left = True

            self.next()

        return name, attrs



# Collect all .scp files in scp2 directory, and run preprocessor.
class Scp2Processor(object):

    tmpin  = "/tmp/parse-scp2.py.cpp"
    tmpout = "/tmp/parse-scp2.py.out"

    def __init__ (self, cur_dir, mod_output_dir):
        self.cur_dir = cur_dir
        self.mod_output_dir = mod_output_dir
        self.scp_files = []
        if not os.path.isfile("%s/scp2/inc/macros.inc"%self.cur_dir):
            raise ParseError("You don't appear to be at the root of OOo's source tree.")
        if not os.path.isdir("%s/scp2/%s/inc"%(self.cur_dir, self.mod_output_dir)):
            raise ParseError("You don't appear to be at the root of OOo's source tree.")

    def run (self):
        # Collect all .scp files under scp2.
        os.path.walk(self.cur_dir + "/scp2", Scp2Processor.visit, self)

        # Process each .scp file.
        for scp in self.scp_files:
            self.process_scp(scp)

    def process_scp (self, scp):
        ret = subprocess.call("cp %s %s"%(scp, Scp2Processor.tmpin), shell=True)
        if ret > 0:
            raise ParseError("failed to copy scp file to a temporary location.")

        subprocess.call("gcc -E -I./scp2/inc -I%s -DUNX %s 2>/dev/null | grep -v -E \"^\#\" > %s"%
            (self.mod_output_dir, Scp2Processor.tmpin, Scp2Processor.tmpout), shell=True)

        file = open(Scp2Processor.tmpout, 'r')
        content = file.read()
        file.close()
        parser = Scp2Parser(content)
        parser.tokenize()
        try:
            parser.parse()
        except ParseError as e:
            error (e.value)
            error ("Error parsing %s"%scp)

    @staticmethod
    def visit (arg, dirname, names):
        instance = arg
        for name in names:
            filepath = dirname + "/" + name
            if os.path.splitext(filepath)[1] == '.scp':
                instance.scp_files.append(filepath)


if __name__ == '__main__':

    processor = optparse.OptionParser()
    processor.usage += " " + arg_desc + "\n" + desc
    processor.add_option("", "--module-output-dir", dest="mod_output_dir", default="unxlngi6.pro", metavar="DIR",
        help="Specify the name of module output directory.  The default value is 'unxlngi6.pro'.")

    options, args = processor.parse_args()
    cur_dir = os.getcwd()
    try:
        processor = Scp2Processor(cur_dir, options.mod_output_dir)
        processor.run()
    except ParseError as e:
        print (e.value)
