import os

import infra.basetest

BASIC_TOOLCHAIN_CONFIG_HEADERS_AT_LEAST_3_14 = \
    """
    LINGMO_arm=y
    LINGMO_TOOLCHAIN_EXTERNAL=y
    LINGMO_TOOLCHAIN_EXTERNAL_CUSTOM=y
    LINGMO_TOOLCHAIN_EXTERNAL_DOWNLOAD=y
    LINGMO_TOOLCHAIN_EXTERNAL_URL="http://autobuild.buildroot.org/toolchains/tarballs/br-arm-full-2019.05.1.tar.bz2"
    LINGMO_TOOLCHAIN_EXTERNAL_GCC_4_9=y
    LINGMO_TOOLCHAIN_EXTERNAL_HEADERS_4_14=y
    LINGMO_TOOLCHAIN_EXTERNAL_LOCALE=y
    # LINGMO_TOOLCHAIN_EXTERNAL_HAS_THREADS_DEBUG is not set
    LINGMO_TOOLCHAIN_EXTERNAL_CXX=y
    """


class TestAtop(infra.basetest.BRTest):
    config = BASIC_TOOLCHAIN_CONFIG_HEADERS_AT_LEAST_3_14 + \
        """
        LINGMO_PACKAGE_ATOP=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv5",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()

        cmd = "atop -V | grep '^Version'"
        self.assertRunOk(cmd)

        cmd = "atop -a 1 2 | grep '% *atop *$'"
        self.assertRunOk(cmd)
