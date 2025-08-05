# main library
libc_mvec = yes

define libc_extra_install
mkdir -p $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH)/mach
mv $(debian-tmp)/usr/include/mach/x86_64 $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH)/mach/
ln -s ../$(DEB_HOST_MULTIARCH)/mach/x86_64 $(debian-tmp)/usr/include/mach/x86_64
endef
