#!/usr/pkg/bin/python2.6
import sys
import re

exp = '#ifndef.*_HXX.*\n(#include.*\n)#endif.*'

filename = sys.argv[1] # warning no error checking

data = open(filename).read()

o = open("/tmp/fixed","w")
o.write( re.sub(exp,"\\1",data) )
o.close()
os.rename("/tmp/fixed", filename)
