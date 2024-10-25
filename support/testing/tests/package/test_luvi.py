import os

import infra.basetest


class TestLuvi(infra.basetest.BRTest):
    config = infra.basetest.BASIC_TOOLCHAIN_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_PACKAGE_LUAJIT=y
        LINGMO_PACKAGE_LUVI=y
        LINGMO_PACKAGE_OPENSSL=y
        LINGMO_PACKAGE_PCRE=y
        LINGMO_PACKAGE_ZLIB=y
        """

    def login(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv5",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()

    def version_test(self):
        cmd = "luvi -v"
        output, exit_code = self.emulator.run(cmd)
        output = sorted(output)
        self.assertIn('libuv', output[0])
        self.assertIn('luvi', output[1])
        self.assertIn('rex', output[2])
        self.assertIn('ssl', output[3])
        self.assertIn('zlib', output[4])

    def test_run(self):
        self.login()
        self.version_test()
