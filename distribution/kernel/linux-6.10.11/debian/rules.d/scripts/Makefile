PROGS = \
	kallsyms \
	recordmcount \
	sign-file \
	unifdef

DATA = \
	Kbuild.include \
	Makefile.* \
	mkversion \
	module-common.lds \
	subarch.include

SCRIPTS =  \
	checkincludes.pl \
	check-local-export \
	checkstack.pl \
	checkversion.pl \
	depmod.sh \
	gcc-*.sh \
	gen_initramfs_list.sh \
	headers_install.sh \
	kernel-doc \
	ld-version.sh \
	Lindent \
	makelst \
	mksysmap \
	mkuboot.sh \
	modules-check.sh \
	namespace.pl \
	pahole-flags.sh \
	pahole-version.sh \
	patch-kernel \
	recordmcount.pl \
	setlocalversion \
	ver_linux

SUBDIRS = \
	basic \
	genksyms \
	kconfig \
	mod

include $(top_rulesdir)/Makefile.inc

CPPFLAGS += -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
sign-file: LDLIBS += -lcrypto
