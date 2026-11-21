#
# Debian packaging helpers. The program itself is built with CMake (see
# README.md); debian/rules drives CMake through debhelper.
#
PKGNAME = asl-parrot
# The single source of truth for the version is VERSION in src/main.cpp.
RELVER := $(shell scripts/parrot-version.sh)
DEBVER = 1
RELPLAT ?= deb$(shell lsb_release -rs 2> /dev/null)

.PHONY: default deb debclean podman-deb

default:
	@echo This does nothing, use 'make podman-deb' or 'make deb'

# Builds the binary package in the current environment (the builder image
# runs this). It downloads the Piper voice and espeak data into etc/ first.
# debian/changelog must already have a $(RELVER)-N entry; an
# autobuild entry for this platform is added for the build and then removed,
# whether or not the build succeeds.
deb:	debclean
	scripts/check-changelog-version.sh
	scripts/fetch-piper-etc.sh
	cp debian/changelog debian/changelog.save
	debchange --distribution stable --package $(PKGNAME) \
		--newversion $(RELVER)-$(DEBVER).$(RELPLAT) \
		"Autobuild of $(RELVER)-$(DEBVER) for $(RELPLAT)"
	dpkg-buildpackage -b --no-sign; rc=$$?; mv debian/changelog.save debian/changelog; exit $$rc

debclean:
	rm -f ../$(PKGNAME)_$(RELVER)*
	rm -f ../$(PKGNAME)-dbgsym_$(RELVER)*
	rm -rf debian/$(PKGNAME)
	rm -rf debian/.debhelper/
	rm -rf obj-*/
	rm -f debian/files
	rm -f debian/debhelper-build-stamp
	rm -f debian/*.substvars
	rm -f debian/*.debhelper
	rm -f debian/*.debhelper.log

# Build the package locally in the builder image with Podman (no publishing).
podman-deb:
	./builder/localbuild.sh
