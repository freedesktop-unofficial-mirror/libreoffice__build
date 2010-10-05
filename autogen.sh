#!/bin/sh
# Run this to generate all the initial makefiles, etc.

if test "z$1" = "z--clean"; then
    echo "Cleaning"

    rm -Rf autom4te.cache bonobo/autom4te.cache
    rm -f missing install-sh mkinstalldirs libtool ltmain.sh
    rm -f bonobo/missing bonobo/install-sh bonobo/mkinstalldirs \
          bonobo/libtool bonobo/ltmain.sh
    exit 1;
fi

requote_args ()
{
    sed -e 's/.*configure //' -e 's/=\(\([^"'"'"'-]\|-[^-]\| \)*\)\( \|$\)/="\1" /g'
}

old_args=""
if test $# -eq 0 && test -f config.log; then
    old_args=`grep '\$ ./configure' config.log | requote_args`
    echo "re-using arguments from last configure: $old_args";
fi

touch ChangeLog

aclocal $ACLOCAL_FLAGS || exit 1;
automake --gnu --add-missing --copy --warnings=no-portability || exit 1;
# intltoolize --copy --force --automake
autoconf || exit 1;
if test "x$NOCONFIGURE" = "x"; then
    if test -n "$old_args" ; then
        eval `echo ./configure $old_args`
    else
        ./configure "$@"
    fi
else
    echo "Skipping configure process."
fi

# prepare git hooks
if [ -d .git ] ; then
    for file in `cd git-hooks ; echo *`
    do
        hook=".git/hooks/$file"
        if [ ! -x "$hook" -a ! -L "$hook" ] ; then
            rm -f "$hook"
            ln -s "../../git-hooks/$file" "$hook"
        fi
    done
fi
