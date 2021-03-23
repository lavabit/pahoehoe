
# daemon - http://libslack.org/daemon/
#
# Copyright (C) 1999-2004, 2010, 2020 raf <raf@raf.org>
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

ifneq ($(DAEMON_TARGET),./$(DAEMON_NAME))

.PHONY: $(DAEMON_NAME)

$(DAEMON_NAME): $(DAEMON_TARGET)

endif

$(DAEMON_TARGET): $(DAEMON_OFILES) $(DAEMON_SUBTARGETS)
	$(CC) -o $(DAEMON_TARGET) $(DAEMON_OFILES) $(DAEMON_LDFLAGS)

.PHONY: man-daemon html-daemon

man-daemon: $(DAEMON_MANFILES)

html-daemon: $(DAEMON_HTMLFILES)

.PHONY: install-daemon install-daemon-bin install-daemon-man install-daemon-html install-daemon-conf

install-daemon: install-daemon-bin install-daemon-man

install-daemon-bin:
	mkdir -p $(APP_INSDIR)
	install -m 755 $(DAEMON_TARGET) $(APP_INSDIR)
	case "$$DEB_BUILD_OPTIONS" in *nostrip*);; *) strip $(patsubst %, $(APP_INSDIR)/%, $(notdir $(DAEMON_TARGET)));; esac

install-daemon-man: man-daemon
	@mkdir -p $(APP_MANDIR); \
	install -m 644 $(DAEMON_MANFILES) $(APP_MANDIR); \
	mkdir -p $(FMT_MANDIR); \
	rm -f $(FMT_MANDIR)/$(notdir $(DAEMON_MANLINK)); \
	ln -s ../man$(APP_MANSECT)/$(notdir $(DAEMON_MANFILES)) $(FMT_MANDIR)/$(notdir $(DAEMON_MANLINK))

install-daemon-html: html-daemon
	@mkdir -p $(DAEMON_HTMLDIR); \
	install -m 644 $(DAEMON_HTMLFILES) $(DAEMON_HTMLDIR)

install-daemon-conf: $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE)
	@mkdir -p $(DAEMON_CONF_INSDIR); \
	[ -f $(DAEMON_CONF_INSDIR)/$(DAEMON_CONFFILE) ] || install -m 644 $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE) $(DAEMON_CONF_INSDIR); \
	[ -d $(DAEMON_CONF_INSDIR)/$(DAEMON_CONFDIR) ] || mkdir $(DAEMON_CONF_INSDIR)/$(DAEMON_CONFDIR)

.PHONY: uninstall-daemon uninstall-daemon-bin uninstall-daemon-man uninstall-daemon-html uninstall-daemon-conf

uninstall-daemon: uninstall-daemon-bin uninstall-daemon-man

uninstall-daemon-bin:
	rm -f $(patsubst %, $(APP_INSDIR)/%, $(notdir $(DAEMON_TARGET)))

uninstall-daemon-man:
	@rm -f $(patsubst %, $(APP_MANDIR)/%, $(notdir $(DAEMON_MANFILES))) \
	$(FMT_MANDIR)/$(notdir $(DAEMON_MANLINK))

uninstall-daemon-html:
	@rm -f $(patsubst %, $(DAEMON_HTMLDIR)/%, $(notdir $(DAEMON_HTMLFILES)))

uninstall-daemon-conf:
	@rm -rf $(DAEMON_CONF_INSDIR)/$(DAEMON_CONFFILE) $(DAEMON_CONF_INSDIR)/$(DAEMON_CONFDIR)

.PHONY: dist-daemon dist-html-daemon rpm-daemon deb-daemon sol-daemon obsd-daemon fbsd-daemon nbsd-daemon osx-daemon

dist-daemon: distclean
	@set -e; \
	up="`pwd`/.."; \
	cd $(DAEMON_SRCDIR); \
	src=`basename \`pwd\``; \
	dst=$(DAEMON_ID); \
	cd ..; \
	[ "$$src" != "$$dst" -a ! -d "$$dst" ] && ln -s $$src $$dst; \
	tar chzf $$up/$(DAEMON_DIST) --exclude='.git*' $$dst; \
	[ -h "$$dst" ] && rm -f $$dst; \
	tar tzfv $$up/$(DAEMON_DIST); \
	ls -l $$up/$(DAEMON_DIST)

dist-html-daemon: html-daemon
	@set -e; \
	up="`pwd`/.."; \
	cd $(DAEMON_SRCDIR); \
	src=`basename \`pwd\``; \
	dst=$(DAEMON_HTML_ID); \
	cd ..; \
	[ "$$src" != "$$dst" -a ! -d "$$dst" ] && ln -s $$src $$dst; \
	tar chzf $$up/$(DAEMON_HTML_DIST) $(patsubst $(DAEMON_SRCDIR)/%, $$dst/%, $(DAEMON_SRCDIR)/README.md $(DAEMON_SRCDIR)/INSTALL $(DAEMON_SRCDIR)/COPYING $(DAEMON_SRCDIR)/LICENSE $(DAEMON_SRCDIR)/REFERENCES $(DAEMON_SRCDIR)/CHANGELOG $(DAEMON_HTMLFILES)); \
	[ -h "$$dst" ] && rm -f $$dst; \
	tar tzfv $$up/$(DAEMON_HTML_DIST); \
	ls -l $$up/$(DAEMON_HTML_DIST)

#RPMDIR := /usr/src/redhat
RPMDIR := $(HOME)/rpmbuild
#RPMBUILD := rpm     # rpm 3.x
RPMBUILD := rpmbuild # rpm 4.x

rpm-daemon: $(DAEMON_SRCDIR)/daemon.spec
	@set -e; \
	up="`pwd`/.."; \
	[ -d $(RPMDIR) ] || mkdir -p $(RPMDIR); \
	[ -d $(RPMDIR)/BUILD ] || mkdir -p $(RPMDIR)/BUILD; \
	[ -d $(RPMDIR)/BUILDROOT ] || mkdir -p $(RPMDIR)/BUILDROOT; \
	[ -d $(RPMDIR)/RPMS ] || mkdir -p $(RPMDIR)/RPMS; \
	[ -d $(RPMDIR)/SOURCES ] || mkdir -p $(RPMDIR)/SOURCES; \
	[ -d $(RPMDIR)/SPECS ] || mkdir -p $(RPMDIR)/SPECS; \
	[ -d $(RPMDIR)/SRPMS ] || mkdir -p $(RPMDIR)/SRPMS; \
	cp $$up/$(DAEMON_DIST) $(RPMDIR)/SOURCES; \
	$(RPMBUILD) --buildroot "/tmp/$(DAEMON_ID)" -ba --target "`uname -m`" $(DAEMON_SRCDIR)/daemon.spec; \
	rm -rf $(DAEMON_SRCDIR)/daemon.spec "/tmp/$(DAEMON_ID)"; \
	mv $(RPMDIR)/SRPMS/$(DAEMON_ID)-*.src.rpm $$up; \
	mv $(RPMDIR)/RPMS/*/$(DAEMON_ID)-*.*.rpm $$up; \
	rm -rf $(RPMDIR)/BUILD/$(DAEMON_ID); \
	rm -f $(RPMDIR)/SOURCES/$(DAEMON_DIST); \
	rm -f $(RPMDIR)/SPECS/daemon.spec; \
	rpm -qlpv $$up/$(DAEMON_ID)-*.*.rpm

DAEMON_SPEC_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		$$summary = $$_ if $$section eq "README" && /^[*\w]/; \
		$$description .= $$_ if $$section eq "DESCRIPTION"; \
		if ($$section ne "README" && $$section ne "DESCRIPTION") \
		{ \
			$$summary =~ s/[*`]//g; \
			$$description =~ s/[*`]//g; \
			print "Summary: $$summary"; \
			print "Name: $(DAEMON_NAME)\n"; \
			print "Version: $(DAEMON_VERSION)\n"; \
			print "Release: 1\n"; \
			print "Group: System Environment/Daemons\n"; \
			print "Source: $(DAEMON_URL)download/$(DAEMON_DIST)\n"; \
			print "URL: $(DAEMON_URL)\n"; \
			print "License: GPL\n"; \
			print "Prefix: $(PREFIX)\n"; \
			print "%description\n"; \
			print $$description; \
			print "%prep\n"; \
			print "%setup\n"; \
			print "%build\n"; \
			print "make\n"; \
			print "%install\n"; \
			print "make PREFIX=\"\$${RPM_BUILD_ROOT}$(PREFIX)\" install-daemon\n"; \
			print "%files\n"; \
			exit; \
		}'

$(DAEMON_SRCDIR)/daemon.spec:
	@set -e; \
	$(DAEMON_SPEC_CODE) < $(DAEMON_SRCDIR)/README.md > $(DAEMON_SRCDIR)/daemon.spec; \
	for file in $(DAEMON_RPM_FILES); do echo $$file >> $(DAEMON_SRCDIR)/daemon.spec; done; \
	for file in $(sort $(DAEMON_RPM_DOCFILES)); do echo %doc $$file >> $(DAEMON_SRCDIR)/daemon.spec; done

# Extra debuild options: Don't sign anything (I'm not a debian maintainer)
DEBUILD_OPTIONS := -us -uc

# Extra debuild options should be:
# DEBUILD_OPTIONS := -mmaintainer@debian.org

deb-daemon: $(DAEMON_SRCDIR)/debian
	@set -e; \
	CDPATH=""; \
	DAEMON_RENAMED=""; \
	case "`pwd`" in \
		*/$(DAEMON_NAME)) \
			cd ..; \
			[ -d $(DAEMON_NAME)-$(DAEMON_VERSION) ] && echo "error: ../$(DAEMON_NAME)-$(DAEMON_VERSION) exists but we're not in it" && exit 1; \
			mv $(DAEMON_NAME) $(DAEMON_NAME)-$(DAEMON_VERSION); \
			cd $(DAEMON_NAME)-$(DAEMON_VERSION); \
			DAEMON_RENAMED="yes";; \
	esac; \
	echo "Building debian packages."; \
	debuild -rfakeroot -tc $(DEBUILD_OPTIONS); \
	rm -rf $(CLEAN_FILES) $(CLOBBER_FILES); \
	[ "$$DAEMON_RENAMED" = "yes" ] && cd .. && mv $(DAEMON_NAME)-$(DAEMON_VERSION) $(DAEMON_NAME) && cd $(DAEMON_NAME); \
	echo "Listing package info and contents."; \
	dpkg --info ../$(DAEMON_NAME)_$(DAEMON_VERSION)-*_*.deb; \
	dpkg --contents ../$(DAEMON_NAME)_$(DAEMON_VERSION)-*_*.deb

DAEMON_DEBIAN_CONTROL_CODE := perl -p -i -e ' \
		BEGIN { \
			open(README, "../README.md") or die("failed to open ../README.md\n"); \
			while (<README>) \
			{ \
				next if /^=+$$/; \
				chop($$section = $$_), next if /^[A-Z]+$$/; \
				$$summary = $$_ if $$section eq "README" && /^[*\w]/; \
				$$description .= $$_ if $$section eq "DESCRIPTION"; \
				if ($$section ne "README" && $$section ne "DESCRIPTION") \
				{ \
					chop($$description); \
					chop($$description); \
					$$description =~ s/[*`]//g; \
					chop($$summary); \
					$$summary = $$1 if $$summary =~ /^\*?\w+\*? - (.*)$$/; \
					$$description =~ s/^$$/./mg; \
					$$description =~ s/^/ /mg; \
					last; \
				} \
			} \
			close(README); \
		} \
		s/^Section: unknown$$/Section: utils/; \
		s/^(Build-Depends: .*)/$$1, perl/; \
		s/^Homepage: <.*>$$/Homepage: http:\/\/libslack.org\/daemon\//; \
		s/, \$$\{misc:Depends\}//; \
		s/^Description: <insert up to 60 chars description>$$/Description: $$summary/; \
		s/^ <insert long description, indented with spaces>$$/$$description/'

DAEMON_DEBIAN_COPYRIGHT_CODE := perl -p -i -e ' \
		s/<url:\/\/example\.com>/<http:\/\/libslack.org\/daemon\/>/; \
		s/Copyright: <years> <put author.s name and email here>/Copyright: 1999-2004, 2010, 2020 raf <raf\@raf.org>/; \
		s/\s+<years> <likewise for another author>\n//; \
		s/License: <special license>/License: GPL-2+/; \
		s/<Put the license of the package here indented by 1 space>/This software is released under the terms of the GNU General Public License v2+:\n\n    http:\/\/www.gnu.org\/copyleft\/gpl.html   (on the Web)\n    file:\/usr\/share\/common-licenses\/GPL-2  (on Debian systems)/; \
		s/ <This follows the format of Description: lines in control file>\n//; \
		s/ <Including paragraphs>\n//; \
		s/\#.*//'

DAEMON_DEBIAN_RULES_CODE := perl -p -i -e ' \
		s/^\t(.*)\$$\(MAKE\) \S*clean$$/\t$$1\$$(MAKE) debian-clobber/; \
		s/^\t\$$\(MAKE\)$$/\t\$$(MAKE) all daemon.conf man-daemon html-daemon/; \
		s/^\t\$$\(MAKE\) .*install.*$$/\t\$$(MAKE) PREFIX=debian\/$(DAEMON_NAME)\/usr install-daemon-bin\n\t\$$(MAKE) PREFIX=debian\/$(DAEMON_NAME)\/usr\/share install-daemon-man\n\t\$$(MAKE) DAEMON_HTMLDIR=debian\/$(DAEMON_NAME)\/usr\/share\/doc\/daemon\/html install-daemon-html\n\t\$$(MAKE) DAEMON_CONF_INSDIR=debian\/$(DAEMON_NAME)\/etc install-daemon-conf/; \
		s/^\tdh_installexamples$$/\#\tdh_installexamples/; \
		s/^\tdh_installmenu$$/\#\tdh_installmenu/; \
		s/^\tdh_installcron$$/\#\tdh_installcron/; \
		s/^\tdh_installman$$/\#\tdh_installman/; \
		s/^\tdh_installinfo$$/\#\tdh_installinfo/; \
		s/^\tdh_suidregister$$/\#\tdh_suidregister/; \
		s/[ \t]+$$//;'

$(DAEMON_SRCDIR)/debian:
	@set -e; \
	CDPATH=""; \
	DAEMON_RENAMED=""; \
	case "`pwd`" in \
		*/$(DAEMON_NAME)) \
			cd ..; \
			[ -d $(DAEMON_NAME)-$(DAEMON_VERSION) ] && echo "error: ../$(DAEMON_NAME)-$(DAEMON_VERSION) exists but we're not in it" && exit 1; \
			mv $(DAEMON_NAME) $(DAEMON_NAME)-$(DAEMON_VERSION); \
			cd $(DAEMON_NAME)-$(DAEMON_VERSION); \
			DAEMON_RENAMED="yes"; \
			;; \
	esac; \
	echo "Creating debian directory."; \
	dh_make -h | grep createorig >/dev/null && createorig=--createorig; \
	dh_make $$createorig -s -e raf@raf.org; \
	[ "$$DAEMON_RENAMED" = "yes" ] && cd .. && mv $(DAEMON_NAME)-$(DAEMON_VERSION) $(DAEMON_NAME) && cd $(DAEMON_NAME); \
	cd $(DAEMON_SRCDIR)/debian; \
	echo "Cleaning up debian directory."; \
	rm -f README.Debian README.source *.ex *.EX ex.*; \
	echo "Creating debian/conffiles."; \
	echo /etc/daemon.conf > conffiles; \
	echo "Completing debian/control."; \
	$(DAEMON_DEBIAN_CONTROL_CODE) control; \
	echo "Completing debian/copyright."; \
	$(DAEMON_DEBIAN_COPYRIGHT_CODE) copyright; \
	echo "Completing debian/changelog."; \
	perl -p -i -e 'last if /^Local variables:/' changelog; \
	perl -p -i -e 's/ \(Closes.*//' changelog; \
	perl -p -i -e 's/-1/-0/' changelog; \
	echo "Creating debian/doc-base."; \
	echo "Document: $(DAEMON_NAME)" > doc-base; \
	echo "Title: $(DAEMON_NAME) manual" >> doc-base; \
	echo "Author: raf <raf@raf.org>" >> doc-base; \
	echo "Abstract: $(DAEMON_NAME)(1) - turns other processes into daemons" >> doc-base; \
	echo "Section: Applications/System" >> doc-base; \
	echo "" >> doc-base; \
	echo "Format: HTML" >> doc-base; \
	echo "Index: /usr/share/doc/$(DAEMON_NAME)/html/" >> doc-base; \
	echo "Files: /usr/share/doc/$(DAEMON_NAME)/html/*.html" >> doc-base; \
	echo "" >> doc-base; \
	echo "Completing debian/rules."; \
	$(DAEMON_DEBIAN_RULES_CODE) rules

.PHONY: debian-clobber

debian-clobber::
	@rm -rf $(CLEAN_FILES) $(DEBIAN_CLOBBER_FILES)

$(DAEMON_SRCDIR)/$(DAEMON_CONFFILE):
	@echo '# /etc/daemon.conf: system-wide daemon(1) configuration.' > $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE); \
	echo '# See daemon(1) for full documentation.' >> $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE); \
	echo '#' >> $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE); \
	echo '# Format: <name|"*"> <option(","option)*>' >> $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE); \
	echo '#' >> $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE); \
	echo '# Examples:' >> $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE); \
	echo '# *      pidfiles=/var/tmp' >> $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE); \
	echo '# name   respawn,command=/usr/bin/command args' >> $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE); \
	echo '' >> $(DAEMON_SRCDIR)/$(DAEMON_CONFFILE)

sol-daemon: $(DAEMON_SRCDIR)/daemon.pkginfo
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p $(DAEMON_SRCDIR)/solaris/install; \
	mkdir -p $(DAEMON_SRCDIR)/solaris/build; \
	mkdir -p $(DAEMON_SRCDIR)/solaris/info; \
	cd $(DAEMON_SRCDIR)/solaris/build; \
	gzip -dc $$up/$(DAEMON_DIST) | tar xf -; \
	cd $(DAEMON_ID); \
	./configure; \
	gmake PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-daemon; \
	cd "$$base"; \
	mv $(DAEMON_SRCDIR)/daemon.pkginfo $(DAEMON_SRCDIR)/solaris/info/pkginfo; \
	cd $(DAEMON_SRCDIR)/solaris/install; \
	pkgproto . > ../info/prototype; \
	echo "i pkginfo" >> ../info/prototype; \
	cd ../info; \
	pkgmk -o -b ../install -r ../install $(DAEMON_SOL); \
	cd "$$base"; \
	rm -rf $(DAEMON_SRCDIR)/solaris; \
	arch="`isainfo -k`"; \
	pkgtrans /var/spool/pkg $(DAEMON_ID).$$arch.pkg $(DAEMON_SOL); \
	rm -rf /var/spool/pkg/$(DAEMON_SOL); \
	mv /var/spool/pkg/$(DAEMON_ID).$$arch.pkg $$up/$(DAEMON_ID)-solaris-$$arch.pkg; \
	gzip $$up/$(DAEMON_ID)-solaris-$$arch.pkg

DAEMON_PKGINFO_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		chop($$description = $$_) if $$section eq "README" && /^[*\w]/; \
		if ($$section ne "README") \
		{ \
			$$description =~ s/[*`]//g; \
			print "PKG=\"$(DAEMON_SOL)\"\n"; \
			print "NAME=\"$$description\"\n"; \
			print "VERSION=\"$(DAEMON_VERSION)\"\n"; \
			print "CATEGORY=\"application\"\n"; \
			print "BASEDIR=\"$(FINAL_PREFIX)\"\n"; \
			exit; \
		}'

$(DAEMON_SRCDIR)/daemon.pkginfo:
	@set -e; \
	$(DAEMON_PKGINFO_CODE) < $(DAEMON_SRCDIR)/README.md > $(DAEMON_SRCDIR)/daemon.pkginfo

obsd-daemon: $(DAEMON_SRCDIR)/obsd-daemon-oneline $(DAEMON_SRCDIR)/obsd-daemon-description
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p "$$base/obsd-$(DAEMON_NAME)/build"; \
	mkdir -p "$$base/obsd-$(DAEMON_NAME)/install"; \
	cd "$$base/obsd-$(DAEMON_NAME)/build"; \
	tar xzf "$$up/$(DAEMON_DIST)"; \
	cd ./$(DAEMON_ID); \
	./configure; \
	gmake PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-daemon; \
	cd "$$base"; \
	echo "@cwd $(PREFIX)" >> $(DAEMON_SRCDIR)/obsd-daemon-packinglist; \
	for file in $(patsubst $(PREFIX)/%, %, $(sort $(DAEMON_RPM_FILES) $(DAEMON_RPM_DOCFILES))); do echo $$file >> $(DAEMON_SRCDIR)/obsd-daemon-packinglist; done; \
	arch="`uname -m`"; \
	pkg_create -A "$$arch" -f $(DAEMON_SRCDIR)/obsd-daemon-packinglist -D COMMENT="`cat $(DAEMON_SRCDIR)/obsd-daemon-oneline`" -d $(DAEMON_SRCDIR)/obsd-daemon-description -p / -v $(DAEMON_ID).tgz; \
	mv $(DAEMON_ID).tgz "$$up/$(DAEMON_ID)-openbsd-$$arch.tgz"; \
	rm -rf "$$base/obsd-$(DAEMON_NAME)" $(DAEMON_SRCDIR)/obsd-daemon-packinglist $(DAEMON_SRCDIR)/obsd-daemon-oneline $(DAEMON_SRCDIR)/obsd-daemon-description

DAEMON_OBSD_ONELINE_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		chop($$description = $$_) if $$section eq "README" && /^[*\w]/; \
		if ($$section ne "README") \
		{ \
			my ($$name, $$desc) = $$description =~ /^\*?(\w+)\*? - (.*)$$/; \
			print "$$desc\n"; \
			exit; \
		}'

$(DAEMON_SRCDIR)/obsd-daemon-oneline:
	@$(DAEMON_OBSD_ONELINE_CODE) < $(DAEMON_SRCDIR)/README.md > $(DAEMON_SRCDIR)/obsd-daemon-oneline

DAEMON_OBSD_DESCRIPTION_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		$$description .= $$_ if $$section eq "DESCRIPTION"; \
		if ($$section ne "README" && $$section ne "DESCRIPTION") \
		{ \
			$$description =~ s/[*`]//g; \
			print $$description; \
			exit; \
		}'

$(DAEMON_SRCDIR)/obsd-daemon-description:
	@$(DAEMON_OBSD_DESCRIPTION_CODE) < $(DAEMON_SRCDIR)/README.md > $(DAEMON_SRCDIR)/obsd-daemon-description

fbsd-daemon: $(DAEMON_SRCDIR)/fbsd-daemon-oneline $(DAEMON_SRCDIR)/fbsd-daemon-description
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p "$$base/fbsd-$(DAEMON_NAME)/build"; \
	mkdir -p "$$base/fbsd-$(DAEMON_NAME)/install"; \
	cd "$$base/fbsd-$(DAEMON_NAME)/build"; \
	tar xzf "$$up/$(DAEMON_DIST)"; \
	cd ./$(DAEMON_ID); \
	./configure; \
	gmake PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-daemon; \
	cd "$$base"; \
	echo "@name $(DAEMON_ID)" > $(DAEMON_SRCDIR)/fbsd-daemon-packinglist; \
	echo "@cwd $(PREFIX)" >> $(DAEMON_SRCDIR)/fbsd-daemon-packinglist; \
	echo "@srcdir $$base/fbsd-$(DAEMON_NAME)/install" >> $(DAEMON_SRCDIR)/fbsd-daemon-packinglist; \
	for file in $(patsubst $(PREFIX)/%, %, $(sort $(DAEMON_RPM_FILES) $(DAEMON_RPM_DOCFILES))); do echo $$file >> $(DAEMON_SRCDIR)/fbsd-daemon-packinglist; done; \
	pkg_create -f $(DAEMON_SRCDIR)/fbsd-daemon-packinglist -c $(DAEMON_SRCDIR)/fbsd-daemon-oneline -d $(DAEMON_SRCDIR)/fbsd-daemon-description -v $(DAEMON_NAME); \
	arch="`uname -m`"; \
	mv $(DAEMON_NAME).tbz "$$up/$(DAEMON_ID)-freebsd-$$arch.tbz"; \
	rm -rf "$$base/fbsd-$(DAEMON_NAME)" $(DAEMON_SRCDIR)/fbsd-daemon-packinglist $(DAEMON_SRCDIR)/fbsd-daemon-oneline $(DAEMON_SRCDIR)/fbsd-daemon-description

DAEMON_FBSD_ONELINE_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		chop($$description = $$_) if $$section eq "README" && /^[*\w]/; \
		if ($$section ne "README") \
		{ \
			my ($$name, $$desc) = $$description =~ /^\*?(\w+)\*? - (.*)$$/; \
			print "$$desc\n"; \
			exit; \
		}'

$(DAEMON_SRCDIR)/fbsd-daemon-oneline:
	@$(DAEMON_FBSD_ONELINE_CODE) < $(DAEMON_SRCDIR)/README.md > $(DAEMON_SRCDIR)/fbsd-daemon-oneline

DAEMON_FBSD_DESCRIPTION_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		$$description .= $$_ if $$section eq "DESCRIPTION"; \
		if ($$section ne "README" && $$section ne "DESCRIPTION") \
		{ \
			$$description =~ s/[*`]//g; \
			print $$description; \
			exit; \
		}'

$(DAEMON_SRCDIR)/fbsd-daemon-description:
	@$(DAEMON_FBSD_DESCRIPTION_CODE) < $(DAEMON_SRCDIR)/README.md > $(DAEMON_SRCDIR)/fbsd-daemon-description

nbsd-daemon: $(DAEMON_SRCDIR)/nbsd-daemon-oneline $(DAEMON_SRCDIR)/nbsd-daemon-description $(DAEMON_SRCDIR)/nbsd-daemon-buildinfo
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p "$$base/nbsd-$(DAEMON_NAME)/build"; \
	mkdir -p "$$base/nbsd-$(DAEMON_NAME)/install"; \
	cd "$$base/nbsd-$(DAEMON_NAME)/build"; \
	tar xzf "$$up/$(DAEMON_DIST)"; \
	cd ./$(DAEMON_ID); \
	./configure; \
	gmake PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-daemon; \
	cd "$$base"; \
	echo "@name $(DAEMON_ID)" > $(DAEMON_SRCDIR)/nbsd-daemon-packinglist; \
	echo "@cwd $(PREFIX)" >> $(DAEMON_SRCDIR)/nbsd-daemon-packinglist; \
	echo "@src $$base/nbsd-$(DAEMON_NAME)/install" >> $(DAEMON_SRCDIR)/nbsd-daemon-packinglist; \
	for file in $(patsubst $(PREFIX)/%, %, $(sort $(DAEMON_RPM_FILES) $(DAEMON_RPM_DOCFILES))); do echo $$file >> $(DAEMON_SRCDIR)/nbsd-daemon-packinglist; done; \
	pkg_create -f $(DAEMON_SRCDIR)/nbsd-daemon-packinglist -c $(DAEMON_SRCDIR)/nbsd-daemon-oneline -d $(DAEMON_SRCDIR)/nbsd-daemon-description -B $(DAEMON_SRCDIR)/nbsd-daemon-buildinfo -v $(DAEMON_NAME); \
	arch="`uname -m`"; \
	mv $(DAEMON_NAME).tgz "$$up/$(DAEMON_ID)-netbsd-$$arch.tgz"; \
	cat $(DAEMON_SRCDIR)/nbsd-daemon-packinglist; \
	rm -rf "$$base/nbsd-$(DAEMON_NAME)" $(DAEMON_SRCDIR)/nbsd-daemon-packinglist $(DAEMON_SRCDIR)/nbsd-daemon-oneline $(DAEMON_SRCDIR)/nbsd-daemon-description $(DAEMON_SRCDIR)/nbsd-daemon-buildinfo

DAEMON_NBSD_ONELINE_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		chop($$description = $$_) if $$section eq "README" && /^[*\w]/; \
		if ($$section ne "README") \
		{ \
			my ($$name, $$desc) = $$description =~ /^\*?(\w+)\*? - (.*)$$/; \
			print "$$desc\n"; \
			exit; \
		}'

$(DAEMON_SRCDIR)/nbsd-daemon-oneline:
	@$(DAEMON_NBSD_ONELINE_CODE) < $(DAEMON_SRCDIR)/README.md > $(DAEMON_SRCDIR)/nbsd-daemon-oneline

DAEMON_NBSD_DESCRIPTION_CODE := perl -ne ' \
		next if /^=+$$/; \
		chop($$section = $$_), next if /^[A-Z]+$$/; \
		$$description .= $$_ if $$section eq "DESCRIPTION"; \
		if ($$section ne "README" && $$section ne "DESCRIPTION") \
		{ \
			$$description =~ s/[*`]//g; \
			print $$description; \
			exit; \
		}'

$(DAEMON_SRCDIR)/nbsd-daemon-description:
	@$(DAEMON_NBSD_DESCRIPTION_CODE) < $(DAEMON_SRCDIR)/README.md > $(DAEMON_SRCDIR)/nbsd-daemon-description

$(DAEMON_SRCDIR)/nbsd-daemon-buildinfo:
	@echo "MACHINE_ARCH=`uname -p`" > $(DAEMON_SRCDIR)/nbsd-daemon-buildinfo; \
	echo "OPSYS=`uname -s`" >> $(DAEMON_SRCDIR)/nbsd-daemon-buildinfo; \
	echo "OS_VERSION=`uname -r`" >> $(DAEMON_SRCDIR)/nbsd-daemon-buildinfo

osx-daemon:
	@set -e; \
	base="`pwd`"; \
	up="$$base/.."; \
	mkdir -p "osx-$(DAEMON_NAME)/build"; \
	mkdir -p "osx-$(DAEMON_NAME)/install"; \
	cd "./osx-$(DAEMON_NAME)/build"; \
	tar xzf "$$up/$(DAEMON_DIST)"; \
	cd ./$(DAEMON_ID); \
	./configure; \
	make PREFIX=../../install FINAL_PREFIX="$(PREFIX)" all install-daemon; \
	cd ../../install; \
	arch="`uname -m`"; \
	tar czf "$$up/$(DAEMON_ID)-macosx-$$arch.tar.gz" .; \
	cd "$$base"; \
	rm -rf "$$base/osx-$(DAEMON_NAME)"

# Present make targets separately in help if we are not alone

ifneq ($(DAEMON_SRCDIR), .)
DAEMON_SPECIFIC_HELP := 1
else
ifneq ($(DAEMON_SUBTARGETS),)
DAEMON_SPECIFIC_HELP := 1
endif
endif

ifeq ($(DAEMON_SPECIFIC_HELP), 1)
help::
	@echo " $(DAEMON_NAME)               -- makes $(DAEMON_TARGET) and $(DAEMON_SUBTARGETS)"; \
	echo " man-$(DAEMON_NAME)           -- makes the $(DAEMON_NAME) manpages"; \
	echo " html-$(DAEMON_NAME)          -- makes the $(DAEMON_NAME) manpages in html"; \
	echo " install-daemon        -- installs $(DAEMON_NAME) and its manpage"; \
	echo " install-daemon-bin    -- installs $(DAEMON_NAME) in $(APP_INSDIR)"; \
	echo " install-daemon-man    -- installs the $(DAEMON_NAME) manpage in $(APP_MANDIR)"; \
	echo " install-daemon-html   -- installs the $(DAEMON_NAME) html manpage in $(DAEMON_HTMLDIR)"; \
	echo " install-daemon-conf   -- installs the $(DAEMON_NAME).conf{,.d} file/directory in $(DAEMON_CONF_INSDIR)"; \
	echo " uninstall-daemon      -- uninstalls $(DAEMON_NAME) and its manpage"; \
	echo " uninstall-daemon-bin  -- uninstalls $(DAEMON_NAME) from $(APP_INSDIR)"; \
	echo " uninstall-daemon-man  -- uninstalls the $(DAEMON_NAME) manpage from $(APP_MANDIR)"; \
	echo " uninstall-daemon-html -- uninstalls the $(DAEMON_NAME) html manpage from $(DAEMON_HTMLDIR)"; \
	echo " uninstall-daemon-conf -- uninstalls the $(DAEMON_NAME).conf{,.d} file/directory from $(DAEMON_CONF_INSDIR)"; \
	echo " dist-daemon           -- makes a source tarball for daemon+libslack"; \
	echo " dist-html-daemon      -- makes a tarball of daemon's html manpages"; \
	echo " rpm-daemon            -- makes source and binary rpm packages for daemon"; \
	echo " deb-daemon            -- makes source and binary deb package for daemon"; \
	echo " sol-daemon            -- makes a binary solaris package for daemon"; \
	echo " obsd-daemon           -- makes a binary openbsd package for daemon"; \
	echo " fbsd-daemon           -- makes a binary freebsd package for daemon"; \
	echo " nbsd-daemon           -- makes a binary netbsd package for daemon"; \
	echo " osx-daemon            -- makes a binary macosx package for daemon"; \
	echo
endif

help-macros::
	@echo "DAEMON_NAME = $(DAEMON_NAME)"; \
	echo "DAEMON_VERSION = $(DAEMON_VERSION)"; \
	echo "DAEMON_ID = $(DAEMON_ID)"; \
	echo "DAEMON_DIST = $(DAEMON_DIST)"; \
	echo "DAEMON_HTML_DIST = $(DAEMON_HTML_DIST)"; \
	echo "DAEMON_TARGET = $(DAEMON_TARGET)"; \
	echo "DAEMON_MODULES = $(DAEMON_MODULES)"; \
	echo "DAEMON_SRCDIR = $(DAEMON_SRCDIR)"; \
	echo "DAEMON_INCDIRS = $(DAEMON_INCDIRS)"; \
	echo "DAEMON_LIBDIRS = $(DAEMON_LIBDIRS)"; \
	echo "DAEMON_LIBS = $(DAEMON_LIBS)"; \
	echo "DAEMON_CFILES = $(DAEMON_CFILES)"; \
	echo "DAEMON_OFILES = $(DAEMON_OFILES)"; \
	echo "DAEMON_HFILES = $(DAEMON_HFILES)"; \
	echo "DAEMON_HTMLDIR = $(DAEMON_HTMLDIR)"; \
	echo "DAEMON_PODFILES = $(DAEMON_PODFILES)"; \
	echo "DAEMON_MANFILES = $(DAEMON_MANFILES)"; \
	echo "DAEMON_HTMLFILES = $(DAEMON_HTMLFILES)"; \
	echo "DAEMON_CONFFILE = $(DAEMON_CONFFILE)"; \
	echo "DAEMON_CONFDIR = $(DAEMON_CONFDIR)"; \
	echo "DAEMON_RPM_FILES = $(DAEMON_RPM_FILES)"; \
	echo "DAEMON_RPM_DOCFILES = $(DAEMON_RPM_DOCFILES)"; \
	echo "DAEMON_DEFINES = $(DAEMON_DEFINES)"; \
	echo "DAEMON_CPPFLAGS = $(DAEMON_CPPFLAGS)"; \
	echo "DAEMON_CCFLAGS = $(DAEMON_CCFLAGS)"; \
	echo "DAEMON_CFLAGS = $(DAEMON_CFLAGS)"; \
	echo "DAEMON_LDFLAGS = $(DAEMON_LDFLAGS)"; \
	echo "DAEMON_SUBTARGETS = $(DAEMON_SUBTARGETS)"; \
	echo "DAEMON_SUBDIRS = $(DAEMON_SUBDIRS)"; \
	echo

include $(SLACK_SRCDIR)/rules.mk

$(DAEMON_SRCDIR)/%.o: $(DAEMON_SRCDIR)/%.c
	$(CC) $(DAEMON_CFLAGS) -o $@ -c $<

ifneq ($(findstring quotes,$(shell $(POD2MAN) --help 2>&1)),)
NOQUOTES := --quotes=none
endif

$(DAEMON_SRCDIR)/%.$(APP_MANSECT): $(DAEMON_SRCDIR)/%.c
	$(POD2MAN) --center='$(APP_MANSECTNAME)' --section=$(APP_MANSECT) $(NOQUOTES) $< > $@

$(DAEMON_SRCDIR)/%.gz: $(DAEMON_SRCDIR)/%
	$(GZIP) $<

$(DAEMON_SRCDIR)/%.$(APP_MANSECT).html: $(DAEMON_SRCDIR)/%.c
	$(POD2HTML) --noindex < $< > $@ 2>/dev/null

