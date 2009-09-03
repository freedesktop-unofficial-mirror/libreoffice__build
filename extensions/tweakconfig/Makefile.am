EXTRA_DIST=META-INF ${wildcard *.xcu} ${wildcard *.xml} 

tweakconfig : $(BUILDDIR)/bin/tweakconfig.oxt

$(BUILDDIR)/bin/tweakconfig.oxt: *.xcu *.xml
	mkdir -p $(BUILDDIR)/bin/tweakconfig && cp -r $(srcdir)/*.xcu $(srcdir)/*.xml META-INF $(BUILDDIR)/bin/tweakconfig/
	awk '{print} /WritePath/ {print "<value>file://$(top_srcdir)/test</value>" }' $(srcdir)/Paths.xcu > $(BUILDDIR)/bin/tweakconfig/Paths.xcu
	cd $(BUILDDIR)/bin/tweakconfig/ && zip -r $@ .

all: tweakconfig

clean-local:
	rm -f $(BUILDDIR)/bin/tweakconfig.oxt
	rm -rf $(BUILDDIR)/bin/tweakconfig
