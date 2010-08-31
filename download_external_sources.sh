#!/bin/sh

. ./bin/setup

export TARFILE_LOCATION
TAG=`echo $OOO_TAG | tr a-z A-Z | sed -e s/-M/_m/`
MWS=`echo $TAG | cut -d_ -f1`
tmp=`mktemp -q -d`

cd $tmp
wget http://hg.services.openoffice.org/${MWS}/raw-file/${TAG}/fetch_tarballs.sh && chmod 755 ./fetch_tarballs.sh
wget http://hg.services.openoffice.org/${MWS}/raw-file/${TAG}/ooo.lst
./fetch_tarballs.sh ooo.lst &&  rm -f fetch_tarballs.sh ooo.lst
cd ..
rmdir $tmp

