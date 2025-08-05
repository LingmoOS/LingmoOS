# Glibc should really do this ld.so link for us.
define libc_extra_install
mkdir -p $(debian-tmp)/lib
ln -s ld.so.1 $(debian-tmp)/lib/ld.so
mkdir -p $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH)/mach
mv $(debian-tmp)/usr/include/mach/i386 $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH)/mach/
ln -s ../$(DEB_HOST_MULTIARCH)/mach/i386 $(debian-tmp)/usr/include/mach/i386
endef
