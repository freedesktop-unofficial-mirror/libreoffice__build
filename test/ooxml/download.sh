#!/bin/sh

SRCDIR=$1

# The test data
cd $SRCDIR/clone
if test -d test-files ;then 
    cd test-files
    git fetch -t origin
else
    # TODO Replace by the ooo-build/contrib URL once the repo is created 
    git clone git://anongit.freedesktop.org/~cbosdo/test-files
fi

# OfficeOTron
# TODO Replace by the SVN copy once the patch is integrated upstream
OFFICEOTRON=officeotron-0.5.5-Beta.jar
if test ! -a $SRCDIR/$OFFICEOTRON ; then
    cd $SRCDIR
    wget "http://people.freedesktop.org/~cbosdo/$OFFICEOTRON" -O $OFFICEOTRON
fi
