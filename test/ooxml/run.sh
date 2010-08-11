#!/bin/sh

ooinstall=$1
TOOLSDIR=$2

. $TOOLSDIR/bin/setup >/dev/null 2>&1

LOGS=`dirname $0`/log


function validate()
{
    FILE_LOG=$LOGS/`basename $1`.log

    mkdir -p `dirname $FILE_LOG`

    SUMMARY_LOG=$LOGS/validations.log
    if test ! -a $SUMMARY_LOG; then
        touch $SUMMARY_LOG
    fi
    java -jar $SRCDIR/officeotron-0.5.5-Beta.jar $1 >$FILE_LOG 2>&1

    # Need to save a log for each file and a summary log
    STATUS="FAILED"
    RESULT=0
    LINES_COUNT='cat $FILE_LOG | wc -l'
    if test "$LINES_COUNT" == "0"; then
        STATUS="PASSED"
        RESULT=1
        rm $FILE_LOG
    fi

    echo -e "$1:\t$STATUS" >>$SUMMARY_LOG
    return $RESULT
}

# Generate the test files
TEST_FILES_DIR=$SRCDIR/clone/test-files
cd $TEST_FILES_DIR && make
cd $OLDPWD

# Validate the test files
RESULT=0
for f in `ls $TEST_FILES_DIR/ooxml-strict/tmp`; do
    validate $TEST_FILES_DIR/ooxml-strict/tmp/$f
    if test $? != 0; then
        RESULT=1
    fi
done


exit $RESULT
