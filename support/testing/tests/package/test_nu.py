import os

import infra.basetest


class TestNu(infra.basetest.BRTest):
    # infra.basetest.BASIC_TOOLCHAIN_CONFIG cannot be used as it doesn't
    # support a host rustc which is necessary for nushell
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_NEON=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_SYSTEM_DHCP="eth0"
        LINGMO_PACKAGE_NUSHELL=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_ROOTFS_POST_BUILD_SCRIPT="{}"
        LINGMO_ROOTFS_POST_SCRIPT_ARGS="{}"
        """.format(infra.filepath("tests/package/copy-sample-script-to-target.sh"),
                   infra.filepath("tests/package/sample_nu.nu"))

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", img])
        self.emulator.login()
        cmd = "nu sample_nu.nu"
        self.assertRunOk(cmd)
