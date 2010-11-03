SUBDIRS = po desktop distro-configs bin etc patches man doc src stamp templates extensions

EXTRA_DIST = \
        autogen.sh              \
	emacs.el		\
	MAINTAINERS		\
	download.in		\
        download_external_sources.sh \
        download_tests_dependencies.sh \
	intltool-merge.in	\
	intltool-update.in	\
	intltool-extract.in

dist-hook:
	git log --date=short --pretty="format:@%cd  %an  <%ae>  [%H]%n%n%s%n%n%e%b" | sed -e "s|^\([^@]\)|\t\1|" -e "s|^@||" >$(distdir)/ChangeLog

clean-local:
	test -z "$(OOBUILDDIR)" || rm -rf $(OOBUILDDIR)
	test -z "$(BUILDDIR)/install" || rm -rf $(BUILDDIR)/install
	for icons in $(OOO_ICONS_VERS) ; do \
	    test "$(BUILDDIR)/$$icons" = "/" || rm -rf "$(BUILDDIR)/$$icons" ; \
	done

maintainer-clean-local:
	test -z "$(BUILDDIR)" || rm -rf $(BUILDDIR)

# If we 'export' all our variables to forked processes
# we end up with a horrible mess - since we propagate
# eg. ACLOCAL_M4, AMDEP_TRUE, COMPATH etc.
# - a nightmare in a nutshell => we need to export on a
#   need-to-know / minimal basis.

STAMP_DIR=$(top_builddir)/stamp

# Source preparation

unpack : $(OOBUILDDIR)/unpack $(top_srcdir)/bin/unpack
$(OOBUILDDIR)/unpack :
	test -n "$(OOO_GIT_MANAGED)" && $(TOOLSDIR)/bin/gob --build-dir=$(OOBUILDDIR) prepare || true
	if test -d $(OOBUILDDIR)/applied_patches ; then \
		FLAGS=`$(TOOLSDIR)/bin/applyflags $(TOOLSDIR)/bin` ; \
		chmod +x $(TOOLSDIR)/patches/apply.pl && $(TOOLSDIR)/patches/apply.pl $(APPLY_DIR) $(OOBUILDDIR) $$FLAGS -f -R ; \
	fi
	cd $(top_srcdir)/bin ; ./unpack
	test -n "$(OOO_GIT_MANAGED)" && $(TOOLSDIR)/bin/gob --build-dir=$(OOBUILDDIR) postpare || true
	rm -f $(STAMP_DIR)/build $(STAMP_DIR)/patch.apply \
	      $(STAMP_DIR)/artwork.install
	touch $@

artwork.install : $(STAMP_DIR)/artwork.install
$(STAMP_DIR)/artwork.install : $(OOBUILDDIR)/unpack \
			      $(STAMP_DIR)/prebuild \
			      $(wildcard $(top_srcdir)/src/*.png) \
			      $(wildcard $(top_srcdir)/src/*.bmp)
	$(TOOLSDIR)/bin/install-artwork $(top_srcdir)/src $(OOBUILDDIR)
	touch $@

patch.apply: $(OOBUILDDIR)/unpack $(STAMP_DIR)/refresh $(STAMP_DIR)/patch.apply
$(STAMP_DIR)/patch.apply : $(top_srcdir)/patches/apply.pl \
			   $(top_srcdir)/patches/*/*.diff \
			   $(APPLY_DIR)/apply
	if test "z$(BUILD_WIN32)" != "z"; then \
	    cp -af $(BUILDDIR)/x86/* $(OOBUILDDIR)/external/dbghelp || true; \
	    cp -af $(BUILDDIR)/*.dll $(OOBUILDDIR)/external/unicows || true; \
	    mv $(OOBUILDDIR)/external/dbghelp/DbgHelp.Dll  \
		     $(OOBUILDDIR)/external/dbghelp/dbghelp.dll.temp || true; \
	    mv $(OOBUILDDIR)/external/dbghelp/dbghelp.dll.temp  \
		     $(OOBUILDDIR)/external/dbghelp/dbghelp.dll || true; \
	    chmod +x $(OOBUILDDIR)/external/dbghelp/*.dll \
		     $(OOBUILDDIR)/external/unicows/*.dll || true; \
	fi

	test -n "$(OOO_GIT_MANAGED)" && $(TOOLSDIR)/bin/gob --build-dir=$(OOBUILDDIR) prepare || true
	FLAGS=`$(TOOLSDIR)/bin/applyflags $(TOOLSDIR)/bin` ; \
	CURSOURCE=$(OOO_SOURCEDIRNAME); test -f $(OOBUILDDIR)/.source && CURSOURCE=`cat $(OOBUILDDIR)/.source`; \
	chmod +x $(TOOLSDIR)/patches/apply.pl && $(TOOLSDIR)/patches/apply.pl $(APPLY_DIR) $(OOBUILDDIR) $$FLAGS --tag=$$CURSOURCE ;
	test -n "$(OOO_GIT_MANAGED)" && $(TOOLSDIR)/bin/gob --build-dir=$(OOBUILDDIR) postpare || true
	test -n "$(OOO_GIT_MANAGED)" && cd $(OOBUILDDIR) && git commit -am 'Font munging.' || true
	$(TOOLSDIR)/bin/fix-deps $(OOBUILDDIR)
	rm -f $(STAMP_DIR)/build
	touch $@

patch.unapply:
	FLAGS=`$(TOOLSDIR)/bin/applyflags $(TOOLSDIR)/bin` ; \
	chmod +x $(TOOLSDIR)/patches/apply.pl && $(TOOLSDIR)/patches/apply.pl $(APPLY_DIR) $(OOBUILDDIR) $$FLAGS -R ;
	rm -f $(STAMP_DIR)/patch.apply

patch.list:
	@FLAGS=`$(TOOLSDIR)/bin/applyflags $(TOOLSDIR)/bin` ; \
	CURSOURCE=$(OOO_SOURCEDIRNAME); test -f $(OOBUILDDIR)/.source && CURSOURCE=`cat $(OOBUILDDIR)/.source`; \
	chmod +x $(TOOLSDIR)/patches/apply.pl && $(TOOLSDIR)/patches/apply.pl $(APPLY_DIR) $(OOBUILDDIR) $$FLAGS --dry-run --quiet --tag=$$CURSOURCE

refresh: $(STAMP_DIR)/refresh
$(STAMP_DIR)/refresh : $(CLONEDIR)/repos_changed
	echo $(OOO_GIT)
	if test "z$(OOO_GIT)" != "z" ; then \
	    FLAGS=`$(TOOLSDIR)/bin/applyflags $(TOOLSDIR)/bin` ; \
	    chmod +x $(TOOLSDIR)/patches/apply.pl && $(TOOLSDIR)/patches/apply.pl $(APPLY_DIR) $(OOBUILDDIR) $$FLAGS -R ; \
	    echo "Rsyncing changes from the repos to the build tree - [ go and have some $(DRINK) ] ..." ; \
	    rsync -prL --exclude .git --exclude 'unxlng??.pro' --exclude 'wntmsci??.pro' --exclude '/solver' --exclude '/install' "$(RAWBUILDDIR)/" "$(OOBUILDDIR)/" ; \
	    rm -f $(STAMP_DIR)/patch.apply ; \
	fi
	touch $@

$(CLONEDIR)/repos_changed :
	if test "z$(OOO_GIT)" != "z" -a ! -e $@ ; then touch $@ ; fi

# Build bits:

prebuild : $(STAMP_DIR)/prebuild
$(STAMP_DIR)/prebuild : $(OOBUILDDIR)/unpack
	echo "Creating environment"
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/bin
	mkdir -p $(BUILDDIR)/share
	mkdir -p $(BUILDDIR)/share/aclocal
	mkdir -p $(BUILDDIR)/share/autoconf
	mkdir -p $(BUILDDIR)/lib
	mkdir -p $(BUILDDIR)/man
	mkdir -p $(BUILDDIR)/man/man1
	mkdir -p $(OOBUILDDIR)
	mkdir -p $(OOBUILDDIR)/external/msvcp71
	touch $@

build.tools : $(STAMP_DIR)/build.tools
$(STAMP_DIR)/build.tools : $(STAMP_DIR)/prebuild
	cd $(top_srcdir)/bin ; ./build-tools
	touch $@

build.prepare : $(STAMP_DIR)/build_prepared
$(STAMP_DIR)/build_prepared : $(OOBUILDDIR)/unpack \
		     $(STAMP_DIR)/refresh \
		     $(STAMP_DIR)/patch.apply \
		     $(STAMP_DIR)/artwork.install \
		     $(STAMP_DIR)/build.tools \
		     $(STAMP_DIR)/prebuild
	touch $@

build : $(STAMP_DIR)/build
$(STAMP_DIR)/build : $(STAMP_DIR)/build_prepared
	cd $(top_srcdir)/bin ; ./build-ooo
	touch $@

all: build

install: $(STAMP_DIR)/build
if BUILD_WIN32
	cd bin ; ./make-win32-iso
else
	cd bin ; ./package-ooo
endif

dev-install: $(BUILDDIR)/install/program/ooenv
$(BUILDDIR)/install/program/ooenv: $(STAMP_DIR)/build
	cd bin; ./ooinstall -l $(BUILDDIR)/install
	sed -i 's/UserInstallation=$$SYSUSERCONFIG\/.libreoffice/UserInstallation=$$ORIGIN\/..\/.libreoffice/g' $(BUILDDIR)/install/program/bootstraprc
	if test -f $(BUILDDIR)/bin/tweakconfig.oxt$(OOBUILDDIR)/applied_patches ; then \
	   $(BUILDDIR)/install/program/unopkg add -v --shared $(BUILDDIR)/bin/tweakconfig.oxt ; \
	fi

# Run smoketest, but timeout in 15 minutes if it got stuck
smoketest: $(BUILDDIR)/install/program/ooenv
	( ( . $(OOBUILDDIR)/*.Set.sh ; export SAL_USE_VCLPLUGIN="svp" ; \
	    cd $(OOBUILDDIR)/test && build.pl && deliver.pl && \
	    cd $(OOBUILDDIR)/smoketestoo_native && build.pl ) & \
	  ( COMMAND_PID=$$! ; DELAY=$$((15*60)) ; \
	    while test "$$DELAY" -gt "0" && ps "$$COMMAND_PID" > /dev/null ; do \
	        sleep 1 ; DELAY=$$(($$DELAY-1)) ; \
	    done ; \
	    ps "$$COMMAND_PID" > /dev/null && ( kill "$$COMMAND_PID" ; echo "Smoketest timed out, killed." ; exit 1 ) ) )

id : $(OOBUILDDIR)/ID
$(OOBUILDDIR)/ID : $(STAMP_DIR)/build
	cd $(OOBUILDDIR); $(TOOLSDIR)/bin/create-ids

tags: $(OOBUILDDIR)/tags
$(OOBUILDDIR)/tags: $(STAMP_DIR)/build_prepared
	cd $(OOBUILDDIR); $(TOOLSDIR)/bin/create-tags

docs: $(BUILDDIR)/docs/index.html
$(BUILDDIR)/docs/index.html: $(STAMP_DIR)/build
	cd $(OOBUILDDIR); $(TOOLSDIR)/bin/mkdocs.sh $(BUILDDIR)/docs $(TOOLSDIR)/bin/doxygen.cfg

cppcheck: $(STAMP_DIR)/build.cppcheck
$(STAMP_DIR)/build.cppcheck: $(STAMP_DIR)/build
	cd $(OOBUILDDIR); $(TOOLSDIR)/bin/mkcppcheck.sh -s -j $(BUILD_NCPUS)

gitignore : $(OOBUILDDIR)/.gitignore
$(OOBUILDDIR)/.gitignore : $(OOBUILDDIR)/unpack
	cd $(OOBUILDDIR); $(TOOLSDIR)/bin/create-gitignores.sh

test: $(BUILDDIR)/install/program/ooenv
	bin/run-tests.sh $(BUILDDIR)/install

vtable_check:
	$(TOOLSDIR)/bin/vtable-check --lo-path=$(OOBUILDDIR)

help:
	@echo "-----------------------------------------------------------------------------------"
	@echo " MAIN TARGETS"
	@echo "-----------------------------------------------------------------------------------"
	@echo ""
	@echo "    * all [Default]:   builds the sources"
	@echo "    * install:         creates the packages or Win32 ISO image"
	@echo "    * clean:           removes the whole build tree"
	@echo ""
	@echo "-----------------------------------------------------------------------------------"
	@echo " DEVELOPEMENT TARGETS"
	@echo "-----------------------------------------------------------------------------------"
	@echo ""
	@echo "    * dev-install:     installs with links to the build tree"
	@echo "    * test:            runs the tests"
	@echo "    * id:              generates the GNU ID database"
	@echo "    * tags:            generates the tags file"
	@echo "    * docs:            generates (partial) doxygen src docs"
	@echo "    * cppcheck:        run cppcheck over (most of) the source tree"
	@echo "    * gitignore:       creates the default .gitignore file in the unpacked sources"
	@echo "    * smoketest:       runs the automated tests"
	@echo "    * patch.apply:     applies the patches to the sources"
	@echo "    * patch.unapply:   revert the applied patches"
	@echo "    * patch.list:      list the applied patches"
	@echo "    * build.prepare:   runs everything before the real build"
