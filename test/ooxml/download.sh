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
if test ! -a $SRCDIR/$OFFICEOTRON ; then
    cd $SRCDIR
    wget "http://people.freedesktop.org/~cbosdo/$OFFICEOTRON" -O $OFFICEOTRON
fi
