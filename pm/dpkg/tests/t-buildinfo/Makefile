OPTS = --admindir=$(CURDIR)/db-buildinfo

test:
	touch test-binary-a_1.0_amd64.deb
	dpkg-source -b pkg-source
	cd pkg-source && DEB_HOST_ARCH=amd64 dpkg-checkbuilddeps $(OPTS)
	cd pkg-source && DEB_HOST_ARCH=amd64 dpkg-genbuildinfo $(OPTS)

test-clean:
	rm -f *.dsc *.deb *.buildinfo
	rm -f *.tar.xz
