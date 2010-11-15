#!/bin/sh

ooinstall=$1
TOOLSDIR=$2

. $TOOLSDIR/bin/setup >/dev/null 2>&1

LOGS=`dirname $0`/log
OUTDIR=`dirname $0`/out

function get_deps()
{
    # The test data
    cd $CLONEDIR
    if test -d test-files ;then
        cd test-files
        git pull -r
    else
        git clone $OOO_GIT/contrib/test-files
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
}

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
    LINES_COUNT=`cat $FILE_LOG | wc -l`
    if test "$LINES_COUNT" == "0"; then
        STATUS="PASSED"
        RESULT=1
        rm $FILE_LOG
    fi

    echo -e "$1:\t$STATUS" >>$SUMMARY_LOG
    return $RESULT
}

# Make sure we have the dependencies
get_deps

# Generate the test files
TEST_FILES_DIR=$CLONEDIR/test-files
cd $TEST_FILES_DIR && make
cd $OLDPWD

# Load and save the test files
soffice.bin -convert-to docx:"Office Open XML Text" -outdir $OUTDIR $TEST_FILES_DIR/ooxml-strict/tmp/*.docx
soffice.bin -convert-to xlsx:"Calc Office Open XML" -outdir $OUTDIR $TEST_FILES_DIR/ooxml-strict/tmp/*.xslx
soffice.bin -convert-to pptx:"Impress Office Open XML" -outdir $OUTDIR $TEST_FILES_DIR/ooxml-strict/tmp/*.pptx

# Validate the test files
RESULT=0
for f in `ls $TEST_FILES_DIR/ooxml-strict/tmp`; do
    validate $TEST_FILES_DIR/ooxml-strict/tmp/$f
    if test $? != 0; then
        RESULT=1
    fi
done


exit $RESULT
