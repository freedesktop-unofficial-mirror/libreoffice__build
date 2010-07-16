#!/bin/sh

# runs all the test scripts in the test folder.
# Args: <install dir>
#
# The main idea:
#   - The script will loop over the test folder subdirectories
#   - If there is a run.sh script inside, then execute it with the 
#     oo install folder as arg.
# 
# This way each script is autonomous and has place to store the logs 
# beside the run.sh script

# Get the oo installation to improve
ooinstall=$1

# Load the whole ooo-build config
. `dirname $0`/setup >/dev/null 2>&1

TESTDIR=$TOOLSDIR/test/

for t in `ls -1 $TESTDIR`
do
    # Is there a run.sh executable script inside?
    if test -x $TESTDIR/$t/run.sh ; then
        sh $TESTDIR/$t/run.sh "$ooinstall" "$TOOLSDIR"
        if test $? ; then
            echo -e "$t\t:\tPASSED"
        else
            echo -e "$t\t:\tFAILED" 
        fi
    else
        echo -e "$t\t:\tSKIPPED"
    fi
done
