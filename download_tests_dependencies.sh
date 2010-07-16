#!/bin/sh

# Loop over the test folders and run the download.sh script if any

echo "Downloading test dependencies"

TESTDIR=./test/

for t in `ls -1 $TESTDIR`
do
    # Is there a run.sh executable script inside?
    if test -x $TESTDIR/$t/download.sh ; then
        echo "Downloading dependencies for test $t"
        sh $TESTDIR/$t/download.sh "$1"
        if test $? != 0 ; then
            echo "Failed to download dependencies for test $t!"
            exit $?
        fi
    fi
done
