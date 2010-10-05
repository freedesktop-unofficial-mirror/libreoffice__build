#!/usr/bin/env python
import sys
import re
import os
import shutil
import tempfile

exp = '#ifndef.*_(?:H|HXX|HRC|HPP)_*[ \t]*\/*.*\n(#include.*\n)#endif[ \t]*\/*.*\n'

filename = sys.argv[1]
if not os.path.isfile(filename):
    print "Error: File does not exist: " + filename
    sys.exit(1)

data = open(filename).read()

temp_dir = tempfile.mkdtemp()
temp_file = temp_dir + "/fixed"

o = open(temp_file,"w")
o.write( re.sub(exp,"\\1",data) )
o.close()

os.chmod(temp_file, 0644)
shutil.move(temp_file, filename)

os.removedirs(temp_dir)
