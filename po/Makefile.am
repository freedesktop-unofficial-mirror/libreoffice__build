SUBDIRS=sdf-templates

EXTRA_DIST=${wildcard *.po} \
	${wildcard *.sdf} \
	lo-build.pot \
	README \
	POTFILES.in

lo_build_po_files = ${wildcard lo-build-*.po}

update-sdf: $(lo_build_po_files:lo-build-%.po=lo-build-%.sdf)

lo-build.sdf: ${wildcard sdf-templates/*.sdf}
	cat $^ | \
	sed -e 's/^\([^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t\)[0-9]*/\1999/' \
	    -e 's/^\([^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t[^\t]*\t\)[-: 0-9]*/\12002-02-02 02:02:02/' | \
	sort -u >$@

lo-build.pot: lo-build.sdf /usr/bin/oo2po
	oo2po -P --multifile=onefile lo-build.sdf $@

lo-build-%.po: lo-build.pot /usr/bin/pot2po
	pot2po -t $@ -i lo-build.pot -o $@.new
	mv $@.new $@

# ../bin/po2sdf is a crazy hack until
# http://bugs.locamotion.org/show_bug.cgi?id=487" is fixed"
lo-build-%.sdf: lo-build-%.po lo-build.sdf /usr/bin/po2oo ../bin/po2sdf
	../bin/po2sdf $* lo-build.sdf $< $@

/usr/bin/oo2po /usr/bin/pot2po /usr/bin/po2po:
	echo "Error: $@ does not exit" ; \
	echo "       You need to install translate-toolkit, see" ; \
	echo "       http://translate.sourceforge.net/wiki/start" ; \
	echo "       The package for openSUSE it named python-translate-toolkit" ; \
	echo "       and can be found at" ; \
	echo "       http://download.opensuse.org/repositories/OpenOffice.org:/EXTRAS/" ; \
	exit 1

clean-local:
	rm -f .intltool-merge-cache
