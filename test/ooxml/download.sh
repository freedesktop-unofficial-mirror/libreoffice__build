#!/bin/sh

SRCDIR=$1

# The test data
cd $SRCDIR/clone
if test -d test-files ;then 
    cd test-files
    git fetch -t origin
else
    git clone git://anongit.freedesktop.org/ooo-build/contrib/test-files
fi

# OfficeOTron
# TODO Replace by the SVN copy once the patch is integrated upstream
#      http://code.google.com/p/officeotron/issues/detail?id=6
OFFICEOTRON=officeotron-0.5.5-Beta.jar
OFFICEOTRONMD5=7b70b7955b7289a8d1502e9c0abf8302
if test ! -a $SRCDIR/$OFFICEOTRON ; then
    cd $SRCDIR
    wget "http://download.go-oo.org/$OFFICEOTRONMD5-$OFFICEOTRON" -O $OFFICEOTRON
fi
