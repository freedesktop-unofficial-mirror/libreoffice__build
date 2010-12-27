#!/bin/sh

. ./bin/setup

export TARFILE_LOCATION
export CLONEDIR

if test -e $CLONEDIR/bootstrap/fetch_tarballs.sh -a -e $CLONEDIR/bootstrap/ooo.lst; then
    $CLONEDIR/bootstrap/fetch_tarballs.sh $CLONEDIR/bootstrap/ooo.lst
else
    tmp=`mktemp -q -d`

    cd $tmp
    wget http://cgit.freedesktop.org/libreoffice/bootstrap/plain/fetch_tarballs.sh?id=$GITTAG -O fetch_tarballs.sh && chmod 755 fetch_tarballs.sh
    wget http://cgit.freedesktop.org/libreoffice/bootstrap/plain/ooo.lst?id=$GITTAG -O ooo.lst
    ./fetch_tarballs.sh ooo.lst &&  rm -f fetch_tarballs.sh ooo.lst
    cd ..
    rmdir $tmp
fi

