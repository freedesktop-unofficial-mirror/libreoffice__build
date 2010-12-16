#!/usr/bin/env sh

ooinstall=$1
TOOLSDIR=$2

. "$TOOLSDIR/bin/setup" >/dev/null 2>&1

RUN_LOG=`dirname "$0"`/run.log
touch "$RUN_LOG"

if test -e "$RUN_LOG"; then
    rm "$RUN_LOG"
fi

LOGS=`dirname "$0"`/log
OUTDIR=`dirname "$0"`/out

function show_progress()
{
    msg="$1: $2%"
    i=0
    backs=
    while test $((i<${#msg})) == 1; do
        backs=$backs
        let i++
    done
    unset i
    echo -en $msg$backs
}

function get_deps()
{
    # The test data
    cd "$CLONEDIR"
    if test -d test-files ;then
        cd test-files
        git pull -r >>"$RUN_LOG" 2>&1
    else
        git clone $OOO_GIT/contrib/test-files >>"$RUN_LOG" 2>&1
    fi

    #OfficeOTron
    cd "$CLONEDIR"
    if test -d officeotron ; then
        cd officeotron
        svn update >>"$RUN_LOG" 2>&1
    else
        svn checkout http://officeotron.googlecode.com/svn/trunk/ officeotron >>"$RUN_LOG" 2>&1
    fi

    # Make / update the officeotrong jar file
    cd "$CLONEDIR/officeotron"
    ant application >>"$RUN_LOG" 2>&1

    # Get the version
    OFFICEOTRON_VERSION=`cat build.xml | grep 'name="version"' | sed -e 's:.*name="version"\ value="\([^"]\+\)".*:\1:'`
    OFFICEOTRON="java -jar $CLONEDIR/officeotron/dist/officeotron-$OFFICEOTRON_VERSION.jar"
}

function validate()
{
    FILE_LOG=$LOGS/`basename $1`.log

    mkdir -p `dirname "$FILE_LOG"`

    SUMMARY_LOG=$LOGS/validations.log
    if test ! -a "$SUMMARY_LOG"; then
        touch "$SUMMARY_LOG"
    fi
    $OFFICEOTRON --errors-only $1 >"$FILE_LOG" 2>&1

    # Need to save a log for each file and a summary log
    STATUS="FAILED"
    RESULT=0
    LINES_COUNT=`cat "$FILE_LOG" | wc -l`
    if test "$LINES_COUNT" == "0"; then
        STATUS="PASSED"
        RESULT=1
        rm "$FILE_LOG"
    fi

    echo -e "$1:\t$STATUS" >>"$SUMMARY_LOG"
    return $RESULT
}

function batch_convert()
{
    MAX_LOT=100
    files_lot=
    lot_size=0
    for file in $4; do
        files_lot+=" $file"
        let lot_size++
        if test "$lot_size" == "$MAX_LOT"; then
            echo "\"$ooinstall/program/soffice\" -infilter=\"$1\" -convert-to \"$2\" -outdir $3 $files_lot" >>"$RUN_LOG"
            "$ooinstall/program/soffice" -infilter="$1" -convert-to "$2" -outdir "$3" $files_lot >>"$RUN_LOG" 2>&1
            files_lot=
            lot_size=0
        fi
    done

    if test "$lot_size" != "0"; then
        echo "\"$ooinstall/program/soffice\" -infilter=\"$1\" -convert-to "$2" -outdir "$3" $files_lot" >>"$RUN_LOG"
        "$ooinstall/program/soffice" -infilter="$1" -convert-to "$2" -outdir "$3" $files_lot >>"$RUN_LOG" 2>&1
    fi
}

# Clean the previous results
if test -d "$OUTDIR"; then
    rm -r "$OUTDIR"
fi
if test -d "$LOGS"; then
    rm -r "$LOGS"
fi

# Make sure we have the dependencies
get_deps

# Generate the test files
TEST_FILES_DIR=$CLONEDIR/test-files
cd "$TEST_FILES_DIR" && make >>"$RUN_LOG" 2>&1
cd "$OLDPWD"

# Load and save the test files
if test -e "$ooinstall/program/ooenv"; then
    . "$ooinstall/program/ooenv"
fi

batch_convert 'Office Open XML Text' 'docx:Office Open XML Text' "$OUTDIR" "$TEST_FILES_DIR/ooxml-strict/tmp/*.docx"
batch_convert 'Calc Office Open XML' 'xlsx:Calc Office Open XML' "$OUTDIR" "$TEST_FILES_DIR/ooxml-strict/tmp/*.xlsx"
batch_convert 'Impress Office Open XML' 'pptx:Impress Office Open XML' "$OUTDIR" "$TEST_FILES_DIR/ooxml-strict/tmp/*.pptx"

# Validate the test files
RESULT=0
out_count=`ls -1 "$OUTDIR" | wc -l`
validated=0
show_progress "Validation" $validated
for f in `ls "$OUTDIR"`; do
    validate "$OUTDIR/$f"
    if test $? != 0; then
        RESULT=1
    fi
    let validated++
    let rate=validated*100/out_count
    show_progress "Validation" $rate
done


exit $RESULT
