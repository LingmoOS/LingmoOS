import os

import infra.basetest


class TestNodeJSBasic(infra.basetest.BRTest):
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_PACKAGE_NODEJS=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_ROOTFS_POST_BUILD_SCRIPT="{}"
        LINGMO_ROOTFS_POST_SCRIPT_ARGS="{}"
        """.format(infra.filepath("tests/package/copy-sample-script-to-target.sh"),
                   infra.filepath("tests/package/sample_nodejs_basic.js"))

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()
        self.assertRunOk("node sample_nodejs_basic.js")


class TestNodeJSModuleHostBin(infra.basetest.BRTest):
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_PACKAGE_NODEJS=y
        LINGMO_PACKAGE_NODEJS_MODULES_ADDITIONAL="lodash"
        LINGMO_PACKAGE_HOST_NODEJS_BIN=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_ROOTFS_POST_BUILD_SCRIPT="{}"
        LINGMO_ROOTFS_POST_SCRIPT_ARGS="{}"
        """.format(infra.filepath("tests/package/copy-sample-script-to-target.sh"),
                   infra.filepath("tests/package/sample_nodejs_module.js"))

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()
        self.assertRunOk("node sample_nodejs_module.js")


class TestNodeJSModuleHostSrc(infra.basetest.BRTest):
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_PACKAGE_NODEJS=y
        LINGMO_PACKAGE_NODEJS_MODULES_ADDITIONAL="lodash"
        LINGMO_PACKAGE_HOST_NODEJS_SRC=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_ROOTFS_POST_BUILD_SCRIPT="{}"
        LINGMO_ROOTFS_POST_SCRIPT_ARGS="{}"
        """.format(infra.filepath("tests/package/copy-sample-script-to-target.sh"),
                   infra.filepath("tests/package/sample_nodejs_module.js"))

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()
        self.assertRunOk("node sample_nodejs_module.js")
