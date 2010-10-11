#!/bin/bash
#
# run cppcheck
#

## subroutines
usage ()
{
	echo "Usage: $0 [options]"
	echo "Options:"
	echo "-j <N>     number of parallel jobs to start"
	echo "-u         try to detect unused function (mutually exclusive to -j)"
	echo "-s         moan about stylistic problems, too"
	echo "-h         this help"
}

## code
NCPUS=1
while getopts hsuj: opt ; do
	case "$opt" in
		j) NCPUS="$OPTARG" ;;
		s) STYLE="--style" ;;
		u) REPORT_UNUSED="--unused-functions" ;;
		h) usage; exit ;;
		?) usage; exit ;;
	esac
done

# binaries that we need
which cppcheck > /dev/null 2>&1 || {
    echo "You need cppcheck for this"
	exit 1
}

# otherwise, aliases are not expanded below
shopt -s expand_aliases

# suck setup
BINDIR=`dirname $0`
. $BINDIR/setup

. ./*.Set.sh

# get list of modules in build order
INPUT_PROJECTS=`cd instsetoo_native && build --all --show | sed -n -e '/Building module/ s/Building module // p'`

# strip -I. and bin -I prefix; exlude system headers
INCLUDE_PATH=`echo $SOLARINC | sed -e ' s/-I\.//'g | sed -e ' s/ -I/ /'g | sed -e ' s|/usr/[^ ]*| |g'`


###################################################
#
# run cppcheck, separately for each module
#
###################################################

for PROJECT in $INPUT_PROJECTS; 
do
  echo "Checking module $PROJECT"
  cppcheck -j$NCPUS $REPORT_UNUSED -v $STYLE $SOLARINC -I$SRC_ROOT/$PROJECT/inc -I$SRC_ROOT/$PROJECT/inc/pch $PROJECT/source
  echo
done

## done
