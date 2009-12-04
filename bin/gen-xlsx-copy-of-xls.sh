#!/bin/bash

TREE="$1"

HEADER="sc/source/filter/inc"
XLS="sc/source/filter/excel"
XLS97="sc/source/filter/xcl97"

if [ ! -d "$TREE/$FILTER" ] ; then
    cat 1>&2 << EOF
gen-xlsx-copy-of-xls.sh build/<milestone>

Updates patches/dev300/xlsx-filter-as-a-separate-lib-xls-copy.diff.
EOF
    exit 1
fi

INC="
excdoc.hxx
excrecds.hxx
exp_op.hxx
XclExpChangeTrack.hxx
xcl97esc.hxx
xcl97rec.hxx
xechart.hxx
xecontent.hxx
xeescher.hxx
xelink.hxx
xename.hxx
xepage.hxx
xepivot.hxx
xerecord.hxx
xestream.hxx
xestyle.hxx
xetable.hxx
xeview.hxx
"

XLS_INC="
xltoolbar.hxx
"

ADD_PREFIX="
colrowst.cxx
excdoc.cxx
excel.cxx
excform.cxx
excform8.cxx
excimp8.cxx
excrecds.cxx
exctools.cxx
expop2.cxx
fontbuff.cxx
frmbase.cxx
impop.cxx
namebuff.cxx
read.cxx
tokstack.cxx
xecontent.cxx
xeescher.cxx
xeformula.cxx
xehelper.cxx
xechart.cxx
xelink.cxx
xename.cxx
xepage.cxx
xepivot.cxx
xerecord.cxx
xeroot.cxx
xestream.cxx
xestring.cxx
xestyle.cxx
xetable.cxx
xeview.cxx
xicontent.cxx
xiescher.cxx
xiformula.cxx
xihelper.cxx
xichart.cxx
xilink.cxx
xiname.cxx
xipage.cxx
xipivot.cxx
xiroot.cxx
xistream.cxx
xistring.cxx
xistyle.cxx
xiview.cxx
xladdress.cxx
xlescher.cxx
xlformula.cxx
xlchart.cxx
xlpage.cxx
xlpivot.cxx
xlroot.cxx
xlstyle.cxx
xltools.cxx
xltoolbar.cxx
xltracer.cxx
xlview.cxx
"

ADD_PREFIX_97="
XclExpChangeTrack.cxx
XclImpChangeTrack.cxx
xcl97esc.cxx
xcl97rec.cxx
"

new-diff() {
    P="$1"
    FROM="$2"
    TO="$3"
    if [ ! -f "$TREE/$P/$FROM" ] ; then
	echo "error: $TREE/$P/$FROM does not exist." 1>&2
	exit 1
    fi
    echo "--- /dev/null"
    echo "+++ sc/source/filter/xlsx/$TO"
    LINES=`wc -l "$TREE/$P/$FROM" | sed 's/ .*//'`
    echo "@@ -0,0 +1,$LINES @@"
    sed 's/^/+/' < "$TREE/$P/$FROM"
}

(
    for I in $INC ; do
        new-diff $HEADER $I $I
    done
    for I in $XLS_INC ; do
        new-diff $XLS $I $I
    done
    for I in $ADD_PREFIX ; do
        new-diff $XLS $I xlsx-$I
    done
    for I in $ADD_PREFIX_97 ; do
        new-diff $XLS97 $I xlsx-$I
    done
) > patches/dev300/xlsx-filter-as-a-separate-lib-xls-copy.diff
