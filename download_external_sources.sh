#!/bin/sh

. ./bin/setup

export TARFILE_LOCATION
export CLONEDIR

if test -e bootstrap/download -a -e bootstrap/ooo.lst; then
    (
        cd bootstrap
        ./download ooo.lst
    )
else
    tmp=`mktemp -q -d`

    cd $tmp
    wget http://cgit.freedesktop.org/libreoffice/bootstrap/plain/fetch_tarballs.sh?id=$OOO_SOURCEDIRNAME -O fetch_tarballs.sh && chmod 755 fetch_tarballs.sh
    wget http://cgit.freedesktop.org/libreoffice/bootstrap/plain/ooo.lst?id=$OOO_SOURCEDIRNAME -O ooo.lst
    ./fetch_tarballs.sh ooo.lst &&  rm -f fetch_tarballs.sh ooo.lst
    cd ..
    rmdir $tmp
fi

