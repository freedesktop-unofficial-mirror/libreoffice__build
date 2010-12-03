#!/bin/bash

# finds undocumented classes in the current directory (recursive)

filter=
quiet=n
if [ "$1" = "-q" ]; then
    filter=">/dev/null"
    quiet=y
    shift
fi

doxygen=$(mktemp -d)
eval doxygen -g $doxygen/doxygen.cfg $filter
sed -i "/HTML_OUTPUT/s|html|$doxygen/html|" $doxygen/doxygen.cfg
sed -i '/GENERATE_LATEX/s/= YES/= NO/' $doxygen/doxygen.cfg
sed -i '/RECURSIVE/s/= NO/= YES/' $doxygen/doxygen.cfg
# do we have any arguments?
if [ -n "$*" ]; then
    sed -i "/^INPUT[^_]/s|=.*|= $*|" $doxygen/doxygen.cfg
fi
eval doxygen $doxygen/doxygen.cfg $filter 2> $doxygen/errors.txt
if [ "$quiet" == "n" ]; then
    echo
    echo "The following classes are undocumented:"
    echo
fi
cat $doxygen/errors.txt|grep -i 'Warning: Compound.*is not documented'
rm -rf $doxygen

# vim:set shiftwidth=4 softtabstop=4 expandtab:
