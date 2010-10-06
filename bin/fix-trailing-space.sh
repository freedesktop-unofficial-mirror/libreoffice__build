#! /bin/bash

# when git commit gives the following, feed this error output as input to this script
# l10ntools/java/jpropex/java/JPropEx.java:28: trailing whitespace.
# +
# l10ntools/scripts/makefile.mk:4: trailing whitespace.
# +#

files=`cat - | grep ': trailing whitespace\.$' | sed 's/:[0-9]*: trailing whitespace.$//' | sort -u`
for file in $files; do
    sed -i 's/\s*$//' $file
done
