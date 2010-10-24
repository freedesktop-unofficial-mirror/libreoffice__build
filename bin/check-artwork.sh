#! /bin/sh

# Take 2 directories with artwork (eg. one reference, 2nd the optimized), and
# check that they do not differ

D1="$1"
D2="$2"

if test -z "$D1" -o -z "$D2" ; then
    echo "check-artwork.sh dir1 dir2"
    exit 1
fi

function same_dir_content ()
{
    SRC="$1"
    DST="$2"
    ( cd "$SRC" ; find . -type f ) | while read F1 ; do
        F2="$DST/$F1"
        if test ! -f "$F2" ; then
            echo "Missing file: '$F2' (source is '$SRC/$F1')"
        fi
    done
}

same_dir_content "$D1" "$D2"
same_dir_content "$D2" "$D1"

# visual comparison using a XPM format
# [couldn't use the oodocdiff.sh trick - usually there are changes in the
# alpha channel that ruin the check :-(]

TEMP=`mktemp -d /tmp/check-artwork.XXXXXXXXXX`
TMP1="$TEMP/1.xpm"
TMP2="$TEMP/2.xpm"
( cd "$D1" ; find . -type f ) | while read F ; do
    convert "$D1/$F" "$TMP1"
    convert "$D2/$F" "$TMP2"
    if diff -q "$TMP1" "$TMP2" > /dev/null 2>&1 ; then
        :
    else
        echo "Images differ: '$D1/$F' and '$D2/$F'"
    fi
done

rm "$TMP1" "$TMP2"
rmdir "$TEMP"
