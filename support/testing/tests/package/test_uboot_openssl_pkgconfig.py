import os

import infra.basetest


class TestUbootOpensslPkgConfig(infra.basetest.BRTest):
    config = infra.basetest.MINIMAL_CONFIG + \
        """
        LINGMO_x86_64=y
        LINGMO_x86_atom=y
        LINGMO_PACKAGE_OPENSSL=y
        LINGMO_TARGET_UBOOT=y
        LINGMO_TARGET_UBOOT_BOARD_DEFCONFIG="efi-x86_payload64"
        LINGMO_TARGET_UBOOT_NEEDS_OPENSSL=y
        """

    def test_run(self):
        img = os.path.join(self.builddir, "images", "u-boot.bin")
        self.assertTrue(os.path.exists(img))
