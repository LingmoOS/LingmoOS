import os

import infra.basetest


class TestStressNG(infra.basetest.BRTest):
    config = infra.basetest.BASIC_TOOLCHAIN_CONFIG + \
        """
        LINGMO_PACKAGE_STRESS_NG=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv5",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()

        self.assertRunOk("stress-ng --version")
        self.assertRunOk("stress-ng --cpu 1 --timeout 3")
