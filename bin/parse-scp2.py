#!/usr/bin/env python

import sys, os, os.path, optparse, subprocess

arg_desc = ""

desc = """
Run this script at the root of OOo source tree."""

def error (msg):
    sys.stderr.write(msg + "\n")

class ErrorBase(Exception):
    
    def __init__ (self, name, msg, sev):
        self.value = "%s: %s"%(name, msg)
        self.sev = sev                    # error severity, 0 = least severe

    def __str__ (self):
        return repr(self.value)

class ParseError(ErrorBase):
    
    def __init__ (self, msg, sev = 0):
        ErrorBase.__init__(self, "ParseError", msg, sev)

class DirError(ErrorBase):
    def __init__ (self, msg):
        ErrorBase.__init__(self, "DirError", msg, 0)

class LinkedNode(object):
    def __init__ (self, name):
        self.name = name
        self.parent = None
        self.children = []

class Scp2Tokenizer(object):

    def __init__ (self, content):
        self.content = content
        self.tokens = []

    def flush_buffer (self):
        if len(self.buf) > 0:
            self.tokens.append(self.buf)
            self.buf = ''

    def run (self):
        self.tokens = []
        i = 0
        n = len(self.content)
        self.buf = ''
        while i < n:
            c = self.content[i]
            if c in '\t\n':
                c = ' '

            if c in ' ;':
                self.flush_buffer()
                if c == ';':
                    self.tokens.append(c)
            elif c == '"':
                # String literal.  Parse until reaching the closing quote.
                self.flush_buffer()
                i += 1
                c = self.content[i]
                while c != '"':
                    self.buf += c
                    i += 1
                    c = self.content[i]
                self.flush_buffer()
            else:
                self.buf += c
            i += 1

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

    def __init__ (self, content, filename):
        self.content = content
        self.filename = filename
        self.nodedata = {}

    def tokenize (self):
        tokenizer = Scp2Tokenizer(self.content)
        tokenizer.run()
        self.tokens = tokenizer.tokens

    def parse (self):
        if len(self.tokens) == 0:
            # No tokens to parse.  Bail out.
            return
            
        self.i = 0
        self.n = len(self.tokens)
        while self.i < self.n:
            t = self.token()
            if t in Scp2Parser.NodeTypes:
                name, attrs, values = self.__parseEntity()
                attrs['__node_type__'] = t                 # type of node
                attrs['__node_location__'] = self.filename # file where the node is defined
                attrs['__node_values__'] = values          # list of values that are not attributes (i.e. not associated with names)
                if self.nodedata.has_key(name):
                    raise ParseError("node named %s already exists"%name, 1)
                self.nodedata[name] = attrs
            else:
                raise ParseError("Unknown node type: %s"%t)

            self.next()

    def append_nodes (self, nodedata, nodetree):

        for key in self.nodedata.keys():

            if nodedata.has_key(key):
                raise ParseError("node named %s already exists"%key, 1)

            # Transfer all the node attributes to the caller instance.
            nodedata[key] = self.nodedata[key]

            # Now, add linkage data to the parent tree instance.

            if not nodetree.has_key(key):
                # Create a new linked node instance.
                nodetree[key] = LinkedNode(key)

            attrs = self.nodedata[key]

            node_type = attrs['__node_type__']
            if node_type == 'Module':
                self.__link_module_node(key, attrs, nodetree)
        

    def __link_module_node (self, name, attrs, nodetree):

        if attrs.has_key('ParentID'):
            parentID = attrs['ParentID']

            if not nodetree.has_key(parentID):
                nodetree[parentID] = LinkedNode(parentID)

            nodetree[parentID].children.append(nodetree[name])
            if nodetree[name].parent != None:
                raise ParseError("parent node instance already exists for '%s'"%parentID, 1)
            nodetree[name].parent = nodetree[parentID]

        if attrs.has_key('Files'):
            # file list strings are formatted '(file1,file2,file3,....)'
            files = attrs['Files']
            if files[0] != '(' or files[-1] != ')':
                raise ParseError("file list string is not formatted correctly: %s"%files)
            files = files[1:-1]
            list = files.split(',')
            for file in list:

                if not nodetree.has_key(file):
                    nodetree[file] = LinkedNode(file)
                nodetree[name].children.append(nodetree[file])


    def next (self):
        self.i += 1

    def token (self):
        return self.tokens[self.i]

    def __parseEntity (self):
        self.next()
        name = self.token()
        if len(name) == 0:
            raise ParseError("empty name", 1)
        left = True
        attr_name = ''
        attr_value = ''
        attrs = {}
        values = []
        self.next()
        while self.token() != 'End':
            if self.token() == '=':
                if not left:
                    raise ParseError("multiple '='s in a single line")

                if len(attr_name) == 0:
                    raise ParseError("empty attribute name")

                left = False
            
            elif left:
                if self.token() == ';':
                    # Not a valid attribute.  Store it as a 'value'.
                    values.append(attr_name)
                    attr_name = ''
                else:
                    attr_name += self.token()
            else:
                # Parse all the way up to ';'
                attr_value = ''
                while self.token() != ';':
                    attr_value += self.token()
                    self.next()
                attrs[attr_name] = attr_value
                left = True
                attr_name = ''

            self.next()

        return name, attrs, values


# Collect all .scp files in scp2 directory, and run preprocessor.
class Scp2Processor(object):

    tmpin  = "/tmp/parse-scp2.py.cpp"
    tmpout = "/tmp/parse-scp2.py.out"

    SkipList = {
        'scp2/source/ooo/ure_standalone.scp': True,
        'scp2/source/sdkoo/sdkoo.scp': True,
        'scp2/source/ooo/starregistry_ooo.scp': True
    }

    def __init__ (self, cur_dir, mod_output_dir):
        self.cur_dir = cur_dir
        self.mod_output_dir = mod_output_dir
        self.scp_files = []
        self.nodedata = {}
        self.nodetree = {}

        # Check file paths first.
        if not os.path.isfile("%s/scp2/inc/macros.inc"%self.cur_dir):
            raise ParseError("You don't appear to be at the root of OOo's source tree.")
        if not os.path.isdir("%s/scp2/%s/inc"%(self.cur_dir, self.mod_output_dir)):
            raise ParseError("You don't appear to be at the root of OOo's source tree.")

    def to_relative (self, fullpath):
        i = fullpath.find("/scp2/")
        if i < 0:
            return fullpath
        i += 1 # skip '/' before 'scp2'
        return fullpath[i:]

    def run (self):
        # Collect all .scp files under scp2.
        os.path.walk(self.cur_dir + "/scp2", Scp2Processor.visit, self)

        # Process each .scp file.
        for scp in self.scp_files:
            relpath = self.to_relative(scp)
            if Scp2Processor.SkipList.has_key(relpath):
                error("skipping %s"%scp)
                continue

            self.process_scp(scp)

    def process_scp (self, scp):
        ret = subprocess.call("cp %s %s"%(scp, Scp2Processor.tmpin), shell=True)
        if ret > 0:
            raise ParseError("failed to copy scp file to a temporary location.")

        subprocess.call("gcc -E -I./scp2/inc -I./scp2/%s/inc -DUNX %s 2>/dev/null | grep -v -E \"^\#\" > %s"%
            (self.mod_output_dir, Scp2Processor.tmpin, Scp2Processor.tmpout), shell=True)

        file = open(Scp2Processor.tmpout, 'r')
        content = file.read()
        file.close()
        parser = Scp2Parser(content, self.to_relative(scp))
        parser.tokenize()
        try:
            parser.parse()
            parser.append_nodes(self.nodedata, self.nodetree)
        except ParseError as e:
            # Skip mal-formed files, instead of exit with error.
            error (e.value)
            error ("Error parsing %s"%scp)
            if e.sev > 0:
                # This is a severe error.  Exit right away.
                sys.exit(1)

    def print_summary_flat (self):
        names = self.nodedata.keys()
        names.sort()
        for name in names:
            attrs = self.nodedata[name]
            node_type = attrs['__node_type__']
            print ('-'*70)
            print ("%s (%s)"%(name, node_type))
            print ("[node location: %s]"%attrs['__node_location__'])

            # Print values first.
            values = attrs['__node_values__']
            for value in values:
                print("  %s"%value)

            # Print all attributes.
            attr_names = attrs.keys()
            attr_names.sort()
            for attr_name in attr_names:
                if attr_name in ['__node_type__', '__node_location__', '__node_values__']:
                    # Skip special attribute.
                    continue
                print ("  %s = %s"%(attr_name, attrs[attr_name]))

    def print_summary_tree (self):
        root = 'gid_Module_Root'
        node = self.nodetree[root]
        self.__print_summary_tree_node(node, 0)

    def __get_fullpath (self, fileID):
        nodedata = self.nodedata[fileID]
        filename = None
        if nodedata.has_key('Name'):
            filename = nodedata['Name']
        elif nodedata.has_key('Name(en-US)'):
            filename = nodedata['Name(en-US)']
        else:
            raise DirError("%s doesn't have a name attribute."%fileID)

        if not nodedata.has_key('Dir'):
            raise DirError("file %s doesn't have Dir attribute."%fileID)

        parent_dir_name = nodedata['Dir']

        while parent_dir_name != None:

            if parent_dir_name == 'PREDEFINED_PROGDIR':
                # special directory name
                return parent_dir_name + '/' + filename

            if not self.nodedata.has_key(parent_dir_name):
                # directory is referenced but not defined.  Skip it for now.
                raise DirError("directory '%s' is referenced but not defined."%parent_dir_name)
    
            nodedata = self.nodedata[parent_dir_name]
            if nodedata.has_key('DosName'):
                filename = nodedata['DosName'] + "/" + filename
            elif nodedata.has_key('DosName(en-US)'):
                filename = nodedata['DosName(en-US)'] + "/" + filename
            elif nodedata.has_key('HostName'):
                filename = nodedata['HostName'] + "/" + filename
            else:
                raise DirError("directory '%s' does not have either DosName or HostName attribute."%parent_dir_name)

            if nodedata.has_key('ParentID'):
                parent_dir_name = nodedata['ParentID']
            else:
                parent_dir_name = None

        return filename

    def __print_summary_tree_node (self, node, level):

        indent = '    '*level

        if node == None:
            return

        if not self.nodedata.has_key(node.name):
            # This node is referenced but is not defined.  Skip it.
            #error("Node '%s' is referenced but not defined."%node.name)
            return

        nodedata = self.nodedata[node.name]
        if not self.nodedata.has_key(node.name):
            raise ParseError("there is no associated node data for '%s'"%node.name)

        node_type = nodedata['__node_type__']

        name = ''
        if node_type == 'File':
            try:
                name = self.__get_fullpath(node.name)
            except DirError as e:
                name = e.value
        elif node_type == 'Directory':
            name = ndoedata['DosName']

        s = indent + "<%s id=\"%s\""%(node_type, node.name)
        if len(name) > 0:
            s += " name=\"%s\""%name

        if len(node.children) > 0:
            s += ">"
            print (s)
    
            children = node.children
            children.sort()
            for child in children:
                self.__print_summary_tree_node(child, level+1)
    
            print (indent + "</%s>"%node_type)
        else:
            s += "/>"
            print (s)

    @staticmethod
    def visit (arg, dirname, names):
        instance = arg
        for name in names:
            filepath = dirname + "/" + name
            if os.path.splitext(filepath)[1] == '.scp':
                instance.scp_files.append(filepath)

if __name__ == '__main__':

    parser = optparse.OptionParser()
    parser.usage += " " + arg_desc + "\n" + desc
    parser.add_option("", "--module-output-dir", dest="mod_output_dir", default="unxlngi6.pro", metavar="DIR",
        help="Specify the name of module output directory.  The default value is 'unxlngi6.pro'.")
    parser.add_option("-m", "--output-mode", dest="mode", default='tree', metavar="MODE",
        help="Specify output mode.  Allowed values are 'tree' and 'flat.  The default mode is 'tree'.")

    options, args = parser.parse_args()

    if not options.mode in ['tree', 'flat']:
        error("unknown output mode '%s'"%options.mode)
        sys.exit(1)

    cur_dir = os.getcwd()
    try:
        processor = Scp2Processor(cur_dir, options.mod_output_dir)
        processor.run()
        if options.mode == 'tree':
            processor.print_summary_tree()
        elif options.mode == 'flat':
            processor.print_summary_flat()
        else:
            raise ParseError("unknown output mode '%s'"%options.mode)

    except ParseError as e:
        error (e.value)
        sys.exit(1)
