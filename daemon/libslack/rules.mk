#
# libslack - http://libslack.org/
#
# Copyright (C) 1999-2002, 2004, 2010, 2020 raf <raf@raf.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.
#
# 20201111 raf <raf@raf.org>

ifneq ($(SLACK_TARGET),./$(SLACK_NAME))

.PHONY: $(SLACK_NAME)

$(SLACK_NAME): $(SLACK_TARGET)

endif

$(SLACK_TARGET): $(SLACK_OFILES)
	$(AR) cr $(SLACK_TARGET) $(SLACK_OFILES)
	$(RANLIB) $(SLACK_TARGET)

.PHONY: ready-slack test-slack man-slack html-slack

ready-slack:
	@[ -h $(SLACK_INCLINK) ] || ln -s . $(SLACK_INCLINK)

test-slack: $(SLACK_TESTS)
	@cd $(SLACK_TESTDIR); for test in $(patsubst $(SLACK_TESTDIR)/%, %, $(SLACK_TESTS)); do echo; ./$$test; done

man-slack: $(SLACK_LIB_MANFILES) $(SLACK_APP_MANFILES)

html-slack: $(SLACK_LIB_HTMLFILES) $(SLACK_APP_HTMLFILES)

.PHONY: install-slack install-slack-bin install-slack-lib install-slack-config install-slack-h install-slack-man install-slack-html

install-slack: install-slack-bin install-slack-h install-slack-man

install-slack-bin: install-slack-lib install-slack-config

install-slack-lib:
	mkdir -p $(LIB_INSDIR)
	install -m 644 $(SLACK_TARGET) $(LIB_INSDIR)/$(SLACK_INSTALL)
	rm -f $(LIB_INSDIR)/$(SLACK_INSTALL_LINK)
	ln -s $(SLACK_INSTALL) $(LIB_INSDIR)/$(SLACK_INSTALL_LINK)

install-slack-config: $(SLACK_CONFIG)
	mkdir -p $(APP_INSDIR)
	install -m 755 $(SLACK_CONFIG) $(APP_INSDIR)

SLACK_CONFIG_CODE := perl -e ' \
		my %arg = \
		( \
			URL       => "$(SLACK_URL)", \
			NAME      => "lib$(SLACK_NAME)", \
			VERSION   => "$(SLACK_VERSION)", \
			PREFIX    => "$(FINAL_PREFIX)", \
			CFLAGS    => "$(strip $(SLACK_CLIENT_CFLAGS))", \
			LIBS      => "$(strip $(SLACK_CLIENT_LDFLAGS))", \
			UNINSTALL => join("", `$(MAKE) -n uninstall-slack | grep rm`) \
		); \
		undef $$/; \
		$$_ = <>; \
		for my $$arg (keys %arg) \
		{ \
			s/\@\@$$arg\@\@/$$arg{$$arg}/g; \
		} \
		print; \
	'

$(SLACK_CONFIG): $(SLACK_CONFIG).t
	@$(SLACK_CONFIG_CODE) < $< > $@

install-slack-h:
	mkdir -p $(HDR_INSDIR)/$(SLACK_NAME)
	install -m 644 $(SLACK_HFILES) $(HDR_INSDIR)/$(SLACK_NAME)

install-slack-man: man-slack
	@mkdir -p $(APP_MANDIR); \
	install -m 644 $(SLACK_APP_MANFILES) $(APP_MANDIR); \
	mkdir -p $(LIB_MANDIR); \
	install -m 644 $(SLACK_LIB_MANFILES) $(LIB_MANDIR); \
	for module in $(SLACK_MODULES); \
	do \
		for func in `perl -n -e 'print $$1, "\n" if /^=item C<(?:const )?\w+[\s*]*(\w+)\(.*\)>$$/ or /^=item C< \#define (\w+)\(.*\)>$$/' "$(SLACK_SRCDIR)/$$module.c"`; \
		do \
			[ -f $(LIB_MANDIR)/$$func.$(LIB_MANSECT)$(MAN_SUFFIX) ] || ln -s $$module.$(LIB_MANSECT)$(MAN_SUFFIX) $(LIB_MANDIR)/$$func.$(LIB_MANSECT)$(MAN_SUFFIX); \
		done; \
	done

install-slack-html: html-slack
	@mkdir -p $(SLACK_HTMLDIR); \
	install -m 644 $(SLACK_APP_HTMLFILES) $(SLACK_LIB_HTMLFILES) $(SLACK_HTMLDIR)

.PHONY: uninstall-slack uninstall-slack-bin uninstall-slack-lib uninstall-slack-config uninstall-slack-h uninstall-slack-man uninstall-slack-html

uninstall-slack: uninstall-slack-bin uninstall-slack-h uninstall-slack-man

uninstall-slack-bin: uninstall-slack-lib uninstall-slack-config

uninstall-slack-lib:
	rm -f $(LIB_INSDIR)/$(SLACK_INSTALL_LINK) $(LIB_INSDIR)/$(SLACK_INSTALL)

uninstall-slack-config:
	rm -f $(patsubst %, $(APP_INSDIR)/%, $(notdir $(SLACK_CONFIG)))

uninstall-slack-h:
	rm -f $(patsubst %, $(HDR_INSDIR)/$(SLACK_NAME)/%, $(notdir $(SLACK_HFILES)))
	rmdir $(HDR_INSDIR)/$(SLACK_NAME) || exit 0

uninstall-slack-man:
	@rm -f $(patsubst %, $(APP_MANDIR)/%, $(notdir $(SLACK_APP_MANFILES)))
	@rm -f $(patsubst %, $(LIB_MANDIR)/%, $(notdir $(SLACK_LIB_MANFILES)))
	@rm -f $(foreach MODULE, $(SLACK_MODULES), $(patsubst %, $(LIB_MANDIR)/%.$(LIB_MANSECT)$(MAN_SUFFIX), $(shell perl -n -e 'print $$1, "\n" if /^=item C<(?:const )?\w+[\s*]*(\w+)\(.*\)>$$/ or /^=item C< \#define (\w+)\(.*\)>$$/' "$(SLACK_SRCDIR)/$(MODULE).c")))

uninstall-slack-html:
	@rm -f $(patsubst %, $(SLACK_HTMLDIR)/%, $(notdir $(SLACK_APP_HTMLFILES) $(SLACK_LIB_HTMLFILES)))

.PHONY: dist-slack dist-html-slack rpm-slack deb-slack sol-slack obsd-slack fbsd-slack nbsd-slack osx-slack

dist-slack: distclean
	@set -e; \
	up="`pwd`/.."; \
	cd $(SLACK_SRCDIR); \
	src=`basename \`pwd\``; \
	dst=$(SLACK_ID); \
	cd ..; \
	[ "$$src" != "$$dst" -a ! -d "$$dst" ] && ln -s $$src $$dst; \
	tar chzf $$up/$(SLACK_DIST) --exclude='.git*' $$dst; \
	[ -h "$$dst" ] && rm -f $$dst; \
	tar tzfv $$up/$(SLACK_DIST); \
	ls -l $$up/$(SLACK_DIST)

dist-html-slack: html-slack
	@set -e; \
	up="`pwd`/.."; \
	cd $(SLACK_SRCDIR); \
	src=`basename \`pwd\``; \
	dst=$(SLACK_HTML_ID); \
	cd ..; \
	[ "$$src" != "$$dst" -a ! -d "$$dst" ] && ln -s $$src $$dst; \
	tar chzf $$up/$(SLACK_HTML_DIST) $(patsubst $(SLACK_SRCDIR)/%, $$dst/%, $(SLACK_SRCDIR)/README.md $(SLACK_SRCDIR)/INSTALL $(SLACK_SRCDIR)/COPYING $(SLACK_SRCDIR)/LICENSE $(SLACK_SRCDIR)/REFERENCES $(SLACK_SRCDIR)/CHANGELOG $(SLACK_LIB_HTMLFILES) $(SLACK_APP_HTMLFILES)); \
	[ -h "$$dst" ] && rm -f $$dst; \
	tar tzfv $$up/$(SLACK_HTML_DIST); \
	ls -l $$up/$(SLACK_HTML_DIST)

#RPMDIR := /usr/src/redhat
RPMDIR := $(HOME)/rpmbuild
#RPMBUILD := rpm     # rpm 3.x
RPMBUILD := rpmbuild # rpm 4.x

rpm-slack: $(SLACK_SRCDIR)/libslack.spec
	@set -e; \
	up="`pwd`/.."; \
	[ -d $(RPMDIR) ] || mkdir -p $(RPMDIR); \
	[ -d $(RPMDIR)/BUILD ] || mkdir -p $(RPMDIR)/BUILD; \
	[ -d $(RPMDIR)/BUILDROOT ] || mkdir -p $(RPMDIR)/BUILDROOT; \
	[ -d $(RPMDIR)/RPMS ] || mkdir -p $(RPMDIR)/RPMS; \
	[ -d $(RPMDIR)/SOURCES ] || mkdir -p $(RPMDIR)/SOURCES; \
	[ -d $(RPMDIR)/SPECS ] || mkdir -p $(RPMDIR)/SPECS; \
	[ -d $(RPMDIR)/SRPMS ] || mkdir -p $(RPMDIR)/SRPMS; \
	cp $$up/$(SLACK_DIST) $(RPMDIR)/SOURCES; \
	$(RPMBUILD) --buildroot "/tmp/$(SLACK_NAME)" -ba --target "`uname -m`" $(SLACK_SRCDIR)/libslack.spec; \
	rm -rf $(SLACK_SRCDIR)/libslack.spec "/tmp/$(SLACK_NAME)"; \
	mv $(RPMDIR)/SRPMS/$(SLACK_ID)-*.src.rpm $$up; \
	mv $(RPMDIR)/RPMS/*/$(SLACK_ID)-*.*.rpm $$up; \
	rm -rf $(RPMDIR)/BUILD/$(SLACK_ID); \
	rm -f $(RPMDIR)/SOURCES/$(SLACK_DIST); \
	rm -f $(RPMDIR)/SPECS/libslack.spec; \
	rpm -qlpv $$up/$(SLACK_ID)-*.*.rpm

SLACK_SPEC_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		$$summary = $$_ if $$section eq "README" && /^[*\w]/; \
		if (/^---/) \
		{ \
			$$summary =~ s/[*`]//g; \
			$$description =~ s/[*`]//g; \
			print "Summary: $$summary"; \
			print "Name: lib$(SLACK_NAME)\n"; \
			print "Version: $(SLACK_VERSION)\n"; \
			print "Release: 1\n"; \
			print "Group: Development/Libraries\n"; \
			print "Source: $(SLACK_URL)download/$(SLACK_DIST)\n"; \
			print "URL: $(SLACK_URL)\n"; \
			print "License: GPL\n"; \
			print "Prefix: $(PREFIX)\n"; \
			print "%description\n"; \
			print $$description; \
			print "%prep\n"; \
			print "%setup\n"; \
			print "%build\n"; \
			print "make\n"; \
			print "%install\n"; \
			print "make PREFIX=\"\$${RPM_BUILD_ROOT}$(PREFIX)\" FINAL_PREFIX=\"$(PREFIX)\" install-slack\n"; \
			print "%files\n"; \
			exit; \
		}; \
		$$description .= $$_ if $$section eq "DESCRIPTION"'

$(SLACK_SRCDIR)/libslack.spec:
	@set -e; \
	$(SLACK_SPEC_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/libslack.spec; \
	for file in $(SLACK_RPM_FILES); do echo $$file >> $(SLACK_SRCDIR)/libslack.spec; done; \
	for file in $(sort $(SLACK_RPM_DOCFILES)); do echo %doc $$file >> $(SLACK_SRCDIR)/libslack.spec; done

deb-slack: $(SLACK_SRCDIR)/libslack.control
	@set -e; \
	pwd="`pwd`"; \
	up="$$pwd/.."; \
	mkdir -p $(SLACK_SRCDIR)/debian/tmp/DEBIAN; \
	mv $(SLACK_SRCDIR)/libslack.control $(SLACK_SRCDIR)/debian/tmp/DEBIAN/control; \
	mkdir -p $(SLACK_SRCDIR)/debian/build; \
	cp $$up/$(SLACK_DIST) $(SLACK_SRCDIR)/debian/build; \
	cd $(SLACK_SRCDIR)/debian/build; \
	tar xzf $(SLACK_DIST); \
	cd ./$(SLACK_ID); \
	make PREFIX=../../tmp/usr FINAL_PREFIX="$(PREFIX)" all install-slack; \
	cd $$pwd; \
	dpkg --build $(SLACK_SRCDIR)/debian/tmp $$up; \
	rm -rf $(SLACK_SRCDIR)/debian; \
	eval "`dpkg-architecture 2>/dev/null`"; \
	dpkg --info $$up/lib$(SLACK_NAME)_$(SLACK_VERSION)_$$DEB_BUILD_ARCH.deb; \
	dpkg --contents $$up/lib$(SLACK_NAME)_$(SLACK_VERSION)_$$DEB_BUILD_ARCH.deb

SLACK_CONTROL_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		$$summary = $$_ if $$section eq "README" && /^[*\w]/; \
		if (/^---/) \
		{ \
			$$description =~ s/^$$/./mg; \
			$$description =~ s/^/ /mg; \
			$$summary =~ s/[*`]//mg; \
			$$description =~ s/[*`]//mg; \
			print "Package: lib$(SLACK_NAME)\n"; \
			print "Version: $(SLACK_VERSION)\n"; \
			print "Maintainer: raf <raf\@raf.org>\n"; \
			print "Section: utils\n"; \
			print "Priority: optional\n"; \
			print "Architecture: '"$$DEB_BUILD_ARCH"'\n"; \
			print "Description: $$summary"; \
			print $$description; \
			exit; \
		}; \
		$$description .= $$_ if $$section eq "DESCRIPTION"'

$(SLACK_SRCDIR)/libslack.control:
	@set -e; \
	eval "`dpkg-architecture 2>/dev/null`"; \
	$(SLACK_CONTROL_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/libslack.control

sol-slack: $(SLACK_SRCDIR)/libslack.pkginfo
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p $(SLACK_SRCDIR)/solaris/install; \
	mkdir -p $(SLACK_SRCDIR)/solaris/build; \
	mkdir -p $(SLACK_SRCDIR)/solaris/info; \
	cd $(SLACK_SRCDIR)/solaris/build; \
	gzip -dc $$up/$(SLACK_DIST) | tar xf -; \
	cd $(SLACK_ID); \
	./configure; \
	gmake PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-slack; \
	cd "$$base"; \
	mv $(SLACK_SRCDIR)/libslack.pkginfo $(SLACK_SRCDIR)/solaris/info/pkginfo; \
	cd $(SLACK_SRCDIR)/solaris/install; \
	pkgproto . > ../info/prototype; \
	echo "i pkginfo" >> ../info/prototype; \
	cd ../info; \
	pkgmk -o -b ../install -r ../install $(SLACK_SOL); \
	cd "$$base"; \
	rm -rf $(SLACK_SRCDIR)/solaris; \
	arch="`isainfo -k`"; \
	pkgtrans /var/spool/pkg $(SLACK_ID).$$arch.pkg $(SLACK_SOL); \
	rm -rf /var/spool/pkg/$(SLACK_SOL); \
	mv /var/spool/pkg/$(SLACK_ID).$$arch.pkg $$up/$(SLACK_ID)-solaris-$$arch.pkg; \
	gzip $$up/$(SLACK_ID)-solaris-$$arch.pkg

SLACK_PKGINFO_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		chop($$description = $$_) if $$section eq "README" && /^[*\w]/; \
		if ($$section ne "README") \
		{ \
			$$description =~ s/[*`]//g; \
			print "PKG=\"$(SLACK_SOL)\"\n"; \
			print "NAME=\"$$description\"\n"; \
			print "VERSION=\"$(SLACK_VERSION)\"\n"; \
			print "CATEGORY=\"application\"\n"; \
			print "BASEDIR=\"$(FINAL_PREFIX)\"\n"; \
			exit; \
		}'

$(SLACK_SRCDIR)/libslack.pkginfo:
	@set -e; \
	$(SLACK_PKGINFO_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/libslack.pkginfo

obsd-slack: $(SLACK_SRCDIR)/obsd-slack-oneline $(SLACK_SRCDIR)/obsd-slack-description
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p "$$base/obsd-$(SLACK_NAME)/build"; \
	mkdir -p "$$base/obsd-$(SLACK_NAME)/install"; \
	cd "$$base/obsd-$(SLACK_NAME)/build"; \
	tar xzf "$$up/$(SLACK_DIST)"; \
	cd ./$(SLACK_ID); \
	./configure; \
	gmake PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-slack; \
	cd "$$base"; \
	echo "@cwd $(PREFIX)" >> $(SLACK_SRCDIR)/obsd-slack-packinglist; \
	for file in $(patsubst $(PREFIX)/%, %, $(sort $(SLACK_RPM_FILES) $(SLACK_RPM_DOCFILES))); do echo $$file >> $(SLACK_SRCDIR)/obsd-slack-packinglist; done; \
	arch="`uname -m`"; \
	pkg_create -A "$$arch" -f $(SLACK_SRCDIR)/obsd-slack-packinglist -D COMMENT="`cat $(SLACK_SRCDIR)/obsd-slack-oneline`" -d $(SLACK_SRCDIR)/obsd-slack-description -p / -v $(SLACK_ID).tgz; \
	mv $(SLACK_ID).tgz "$$up/$(SLACK_ID)-openbsd-$$arch.tgz"; \
	rm -rf "$$base/obsd-$(SLACK_NAME)" $(SLACK_SRCDIR)/obsd-slack-packinglist $(SLACK_SRCDIR)/obsd-slack-oneline $(SLACK_SRCDIR)/obsd-slack-description

SLACK_OBSD_ONELINE_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		chop($$description = $$_) if $$section eq "README" && /^[*\w]/; \
		if ($$section ne "README") \
		{ \
			$$description =~ s/[*`]//g; \
			my ($$name, $$desc) = $$description =~ /^\*?(\w+)\*? - (.*)$$/; \
			$$desc =~ s/general //; \
			print "$$desc\n"; \
			exit; \
		}'

$(SLACK_SRCDIR)/obsd-slack-oneline:
	@$(SLACK_OBSD_ONELINE_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/obsd-slack-oneline

SLACK_OBSD_DESCRIPTION_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		if (/^---/) \
		{ \
			$$description =~ s/[*`]//g; \
			print $$description; \
			exit; \
		}; \
		$$description .= $$_ if $$section eq "DESCRIPTION"'

$(SLACK_SRCDIR)/obsd-slack-description:
	@$(SLACK_OBSD_DESCRIPTION_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/obsd-slack-description

fbsd-slack: $(SLACK_SRCDIR)/fbsd-slack-oneline $(SLACK_SRCDIR)/fbsd-slack-description
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p "$$base/fbsd-$(SLACK_NAME)/build"; \
	mkdir -p "$$base/fbsd-$(SLACK_NAME)/install"; \
	cd "$$base/fbsd-$(SLACK_NAME)/build"; \
	tar xzf "$$up/$(SLACK_DIST)"; \
	cd ./$(SLACK_ID); \
	./configure; \
	gmake PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-slack; \
	cd "$$base"; \
	echo "@name $(SLACK_ID)" > $(SLACK_SRCDIR)/fbsd-slack-packinglist; \
	echo "@cwd $(PREFIX)" >> $(SLACK_SRCDIR)/fbsd-slack-packinglist; \
	echo "@srcdir $$base/fbsd-$(SLACK_NAME)/install" >> $(SLACK_SRCDIR)/fbsd-slack-packinglist; \
	for file in $(patsubst $(PREFIX)/%, %, $(sort $(SLACK_RPM_FILES) $(SLACK_RPM_DOCFILES))); do echo $$file >> $(SLACK_SRCDIR)/fbsd-slack-packinglist; done; \
	echo "@dirrm include/slack" >> $(SLACK_SRCDIR)/fbsd-slack-packinglist; \
	pkg_create -f $(SLACK_SRCDIR)/fbsd-slack-packinglist -c $(SLACK_SRCDIR)/fbsd-slack-oneline -d $(SLACK_SRCDIR)/fbsd-slack-description -v $(SLACK_NAME); \
	arch="`uname -m`"; \
	mv $(SLACK_NAME).tbz "$$up/$(SLACK_ID)-freebsd-$$arch.tbz"; \
	rm -rf "$$base/fbsd-$(SLACK_NAME)" $(SLACK_SRCDIR)/fbsd-slack-packinglist $(SLACK_SRCDIR)/fbsd-slack-oneline $(SLACK_SRCDIR)/fbsd-slack-description

SLACK_FBSD_ONELINE_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		chop($$description = $$_) if $$section eq "README" && /^[*\w]/; \
		if ($$section ne "README") \
		{ \
			my ($$name, $$desc) = $$description =~ /^\*?(\w+)\*? - (.*)$$/; \
			$$desc =~ s/general //; \
			$$desc =~ s/[*`]//g; \
			print "$$desc\n"; \
			exit; \
		}'

$(SLACK_SRCDIR)/fbsd-slack-oneline:
	@$(SLACK_FBSD_ONELINE_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/fbsd-slack-oneline

SLACK_FBSD_DESCRIPTION_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		if (/^---/) \
		{ \
			$$description =~ s/[*`]//g; \
			print $$description; \
			exit; \
		}; \
		$$description .= $$_ if $$section eq "DESCRIPTION"'

$(SLACK_SRCDIR)/fbsd-slack-description:
	@$(SLACK_FBSD_DESCRIPTION_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/fbsd-slack-description

nbsd-slack: $(SLACK_SRCDIR)/nbsd-slack-oneline $(SLACK_SRCDIR)/nbsd-slack-description $(SLACK_SRCDIR)/nbsd-slack-buildinfo
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p "$$base/nbsd-$(SLACK_NAME)/build"; \
	mkdir -p "$$base/nbsd-$(SLACK_NAME)/install"; \
	cd "$$base/nbsd-$(SLACK_NAME)/build"; \
	tar xzf "$$up/$(SLACK_DIST)"; \
	cd ./$(SLACK_ID); \
	./configure; \
	gmake PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-slack; \
	cd "$$base"; \
	echo "@name $(SLACK_ID)" > $(SLACK_SRCDIR)/nbsd-slack-packinglist; \
	echo "@cwd $(PREFIX)" >> $(SLACK_SRCDIR)/nbsd-slack-packinglist; \
	echo "@src $$base/nbsd-$(SLACK_NAME)/install" >> $(SLACK_SRCDIR)/nbsd-slack-packinglist; \
	for file in $(patsubst $(PREFIX)/%, %, $(sort $(SLACK_RPM_FILES) $(SLACK_RPM_DOCFILES))); do echo $$file >> $(SLACK_SRCDIR)/nbsd-slack-packinglist; done; \
	pkg_create -f $(SLACK_SRCDIR)/nbsd-slack-packinglist -c $(SLACK_SRCDIR)/nbsd-slack-oneline -d $(SLACK_SRCDIR)/nbsd-slack-description -B $(SLACK_SRCDIR)/nbsd-slack-buildinfo -v $(SLACK_NAME); \
	arch="`uname -m`"; \
	mv $(SLACK_NAME).tgz "$$up/$(SLACK_ID)-netbsd-$$arch.tgz"; \
	rm -rf "$$base/nbsd-$(SLACK_NAME)" $(SLACK_SRCDIR)/nbsd-slack-packinglist $(SLACK_SRCDIR)/nbsd-slack-oneline $(SLACK_SRCDIR)/nbsd-slack-description $(SLACK_SRCDIR)/nbsd-slack-buildinfo

SLACK_NBSD_ONELINE_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		chop($$description = $$_) if $$section eq "README" && /^[*\w]/; \
		if ($$section ne "README") \
		{ \
			my ($$name, $$desc) = $$description =~ /^\*?(\w+)\*? - (.*)$$/; \
			$$desc =~ s/[*`]//g; \
			$$desc =~ s/general //; \
			print "$$desc\n"; \
			exit; \
		}'

$(SLACK_SRCDIR)/nbsd-slack-oneline:
	@$(SLACK_NBSD_ONELINE_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/nbsd-slack-oneline

SLACK_NBSD_DESCRIPTION_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		if (/^---/) \
		{ \
			$$description =~ s/[*`]//g; \
			print $$description; \
			exit; \
		}; \
		$$description .= $$_ if $$section eq "DESCRIPTION"'

$(SLACK_SRCDIR)/nbsd-slack-description:
	@$(SLACK_NBSD_DESCRIPTION_CODE) < $(SLACK_SRCDIR)/README.md > $(SLACK_SRCDIR)/nbsd-slack-description

$(SLACK_SRCDIR)/nbsd-slack-buildinfo:
	@echo "MACHINE_ARCH=`uname -p`" > $(SLACK_SRCDIR)/nbsd-slack-buildinfo; \
	echo "OPSYS=`uname -s`" >> $(SLACK_SRCDIR)/nbsd-slack-buildinfo; \
	echo "OS_VERSION=`uname -r`" >> $(SLACK_SRCDIR)/nbsd-slack-buildinfo

osx-slack:
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p "osx-$(SLACK_NAME)/build"; \
	mkdir -p "osx-$(SLACK_NAME)/install"; \
	cd "./osx-$(SLACK_NAME)/build"; \
	tar xzf "$$up/$(SLACK_DIST)"; \
	cd ./$(SLACK_ID); \
	./configure; \
	make PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-slack; \
	cd ../../install; \
	arch="`uname -m`"; \
	tar czf "$$up/$(SLACK_ID)-macosx-$$arch.tar.gz" .; \
	cd "$$base"; \
	rm -rf "$$base/osx-$(SLACK_NAME)"

# Present make targets separately in help if we are not alone

ifneq ($(SLACK_SRCDIR), .)
SLACK_SPECIFIC_HELP := 1
else
ifneq ($(SLACK_SUBTARGETS),)
SLACK_SPECIFIC_HELP := 1
endif
endif

ifeq ($(SLACK_SPECIFIC_HELP), 1)
help::
	@echo " $(SLACK_NAME)                 -- makes $(SLACK_TARGET)"; \
	echo " ready-$(SLACK_NAME)           -- prepares the source directory for make"; \
	echo " man-$(SLACK_NAME)             -- makes the $(SLACK_NAME) manpages"; \
	echo " html-$(SLACK_NAME)            -- makes the $(SLACK_NAME) manpages in html"; \
	echo " install-slack         -- installs $(SLACK_NAME), headers and manpages"; \
	echo " install-slack-bin     -- installs $(SLACK_NAME) in $(LIB_INSDIR)"; \
	echo " install-slack-h       -- installs $(SLACK_NAME) headers in $(HDR_INSDIR)/$(SLACK_NAME)"; \
	echo " install-slack-man     -- installs $(SLACK_NAME) manpages in $(LIB_MANDIR)"; \
	echo " install-slack-html    -- installs $(SLACK_NAME) html manpages in $(SLACK_HTMLDIR)"; \
	echo " uninstall-slack       -- uninstalls $(SLACK_NAME), its headers and manpages"; \
	echo " uninstall-slack-bin   -- uninstalls $(SLACK_NAME) from $(LIB_INSDIR)"; \
	echo " uninstall-slack-h     -- uninstalls $(SLACK_NAME) headers from $(HDR_INSDIR)/$(SLACK_NAME)"; \
	echo " uninstall-slack-man   -- uninstalls $(SLACK_NAME) manpages from $(LIB_MANDIR)"; \
	echo " uninstall-slack-html  -- uninstalls $(SLACK_NAME) html manpages from $(SLACK_HTMLDIR)"; \
	echo " test-slack            -- makes and runs library unit tests"; \
	echo " dist-slack            -- makes a source tarball for libslack"; \
	echo " dist-html-slack       -- makes a tarball of libslack's html manpages"; \
	echo " rpm-slack             -- makes binary and source rpm packages for libslack"; \
	echo " deb-slack             -- makes a binary deb package for libslack"; \
	echo " sol-slack             -- makes a binary solaris pkg for libslack"; \
	echo " obsd-slack            -- makes a binary openbsd pkg for libslack"; \
	echo " fbsd-slack            -- makes a binary freebsd pkg for libslack"; \
	echo " nbsd-slack            -- makes a binary netbsd pkg for libslack"; \
	echo " osx-slack             -- makes a binary macosx pkg for libslack"; \
	echo " slack.swig            -- makes a SWIG input file for libslack"; \
	echo
endif

help-macros::
	@echo "SLACK_NAME = $(SLACK_NAME)"; \
	echo "SLACK_VERSION = $(SLACK_VERSION)"; \
	echo "SLACK_ID = $(SLACK_ID)"; \
	echo "SLACK_DIST = $(SLACK_DIST)"; \
	echo "SLACK_HTML_DIST = $(SLACK_HTML_DIST)"; \
	echo "SLACK_TARGET = $(SLACK_TARGET)"; \
	echo "SLACK_INSTALL = $(SLACK_INSTALL)"; \
	echo "SLACK_CONFIG = $(SLACK_CONFIG)"; \
	echo "SLACK_MODULES = $(SLACK_MODULES)"; \
	echo "SLACK_HEADERS = $(SLACK_HEADERS)"; \
	echo "SLACK_SRCDIR = $(SLACK_SRCDIR)"; \
	echo "SLACK_INCDIRS = $(SLACK_INCDIRS)"; \
	echo "SLACK_LIBDIRS = $(SLACK_LIBDIRS)"; \
	echo "SLACK_TESTDIR = $(SLACK_TESTDIR)"; \
	echo "SLACK_CFILES = $(SLACK_CFILES)"; \
	echo "SLACK_OFILES = $(SLACK_OFILES)"; \
	echo "SLACK_HFILES = $(SLACK_HFILES)"; \
	echo "SLACK_PODNAMES = $(SLACK_PODNAMES)"; \
	echo "SLACK_HTMLDIR = $(SLACK_HTMLDIR)"; \
	echo "SLACK_LIB_MANFILES = $(SLACK_LIB_MANFILES)"; \
	echo "SLACK_APP_MANFILES = $(SLACK_APP_MANFILES)"; \
	echo "SLACK_SWIGFILE = $(SLACK_SWIGFILE)"; \
	echo "SLACK_LIB_HTMLFILES = $(SLACK_LIB_HTMLFILES)"; \
	echo "SLACK_APP_HTMLFILES = $(SLACK_APP_HTMLFILES)"; \
	echo "SLACK_RPM_FILES = $(SLACK_RPM_FILES)"; \
	echo "SLACK_RPM_DOCFILES = $(SLACK_RPM_DOCFILES)"; \
	echo "SLACK_TESTS = $(SLACK_TESTS)"; \
	echo "SLACK_DEFINES = $(SLACK_DEFINES)"; \
	echo "SLACK_CPPFLAGS = $(SLACK_CPPFLAGS)"; \
	echo "SLACK_CCFLAGS = $(SLACK_CCFLAGS)"; \
	echo "SLACK_CFLAGS = $(SLACK_CFLAGS)"; \
	echo "SLACK_TEST_CPPFLAGS = $(SLACK_TEST_CPPFLAGS)"; \
	echo "SLACK_TEST_CCFLAGS = $(SLACK_TEST_CCFLAGS)"; \
	echo "SLACK_TEST_CFLAGS = $(SLACK_TEST_CFLAGS)"; \
	echo "SLACK_TEST_LIBS = $(SLACK_TEST_LIBS)"; \
	echo "SLACK_TEST_LDFLAGS = $(SLACK_TEST_LDFLAGS)"; \
	echo "SLACK_CLIENT_CFLAGS = $(SLACK_CLIENT_CFLAGS)"; \
	echo "SLACK_CLIENT_LIBS = $(SLACK_CLIENT_LIBS)"; \
	echo "SLACK_CLIENT_LDFLAGS = $(SLACK_CLIENT_LDFLAGS)"; \
	echo "SLACK_SUBTARGETS = $(SLACK_SUBTARGETS)"; \
	echo "SLACK_SUBDIRS = $(SLACK_SUBDIRS)"; \
	echo

distclean::
	@rm -f $(SLACK_INCLINK)

$(SLACK_SRCDIR)/%.o: $(SLACK_SRCDIR)/%.c
	$(CC) $(SLACK_CFLAGS) -o $@ -c $<

$(SLACK_TESTDIR)/%: $(SLACK_SRCDIR)/%.c $(SLACK_TARGET)
	@[ -d $(SLACK_TESTDIR) ] || mkdir $(SLACK_TESTDIR)
	$(CC) -DTEST $(SLACK_TEST_CFLAGS) -o $@ $< $(SLACK_TEST_LDFLAGS)

ifneq ($(findstring quotes,$(shell $(POD2MAN) --help 2>&1)),)
NOQUOTES := --quotes=none
endif

$(SLACK_SRCDIR)/%.$(LIB_MANSECT): $(SLACK_SRCDIR)/%.c
	$(POD2MAN) --center='$(LIB_MANSECTNAME)' --section=$(LIB_MANSECT) $(NOQUOTES) $< > $@

$(SLACK_SRCDIR)/%.$(LIB_MANSECT): $(SLACK_SRCDIR)/%.pod
	$(POD2MAN) --center='$(LIB_MANSECTNAME)' --section=$(LIB_MANSECT) $(NOQUOTES) $< > $@

$(SLACK_SRCDIR)/%.$(APP_MANSECT): $(SLACK_SRCDIR)/%.pod
	$(POD2MAN) --center='$(APP_MANSECTNAME)' --section=$(APP_MANSECT) $(NOQUOTES) $< > $@

$(SLACK_SRCDIR)/%.gz: $(SLACK_SRCDIR)/%
	$(GZIP) $<

$(SLACK_SRCDIR)/%.$(LIB_MANSECT).html: $(SLACK_SRCDIR)/%.c
	$(POD2HTML) --noindex < $< > $@ 2>/dev/null
	@perl -p -i -e 's/\&amp;lt;/\&lt;/g;s/\&amp;gt;/\&gt;/g' $@

$(SLACK_SRCDIR)/%.$(LIB_MANSECT).html: $(SLACK_SRCDIR)/%.pod
	$(POD2HTML) --noindex < $< > $@ 2>/dev/null

$(SLACK_SRCDIR)/%.$(APP_MANSECT).html: $(SLACK_SRCDIR)/%.pod
	$(POD2HTML) --noindex < $< > $@ 2>/dev/null

SLACK_SWIGFILE_CODE := perl -e ' \
		print "%module slack\n"; \
		print "%{\n"; \
		print "\#include \"$(PREFIX)/include/slack/lib.h\"\n"; \
		print "%}\n\n"; \
		for (split /\s+/, "$(SLACK_CLIENT_CFLAGS)") \
		{ \
			s/^-D//; s/=/ /; \
			print "\#define $$_\n"; \
		} \
		print "\n%import $(PREFIX)/include/slack/hdr.h\n\n"; \
		for (@ARGV) \
		{ \
			print "%include $(PREFIX)/include/slack/$$_.h\n"; \
		}'

$(SLACK_SWIGFILE): $(SLACK_HFILES)
	@$(SLACK_SWIGFILE_CODE) $(SLACK_MODULES) > $(SLACK_SWIGFILE)

