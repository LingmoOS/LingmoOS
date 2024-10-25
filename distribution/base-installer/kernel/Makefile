ARCHES = alpha amd64 arm64 armel armhf hppa i386 ia64 m68k mips mipsel mips64el powerpc ppc64 ppc64el s390x sh4 sparc sparc64 kfreebsd-amd64 kfreebsd-i386

test:
	@STATUS=0; \
	cd tests && \
	for x in $(ARCHES); do ./runtests "$$x" || STATUS=1; done; \
	exit "$$STATUS"
