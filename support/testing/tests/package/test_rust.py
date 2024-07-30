import os
import shutil

import infra.basetest


class TestRustBase(infra.basetest.BRTest):

    def login(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", img])
        self.emulator.login()


class TestRustBin(TestRustBase):
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_NEON=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_SYSTEM_DHCP="eth0"
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_PACKAGE_HOST_RUSTC=y
        LINGMO_PACKAGE_RIPGREP=y
        """

    def test_run(self):
        self.login()
        self.assertRunOk("rg Buildroot /etc/issue")


class TestRust(TestRustBase):
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_NEON=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_SYSTEM_DHCP="eth0"
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_PACKAGE_HOST_RUSTC=y
        LINGMO_PACKAGE_HOST_RUST=y
        LINGMO_PACKAGE_RIPGREP=y
        """

    def test_run(self):
        self.login()
        self.assertRunOk("rg Buildroot /etc/issue")


class TestRustVendoring(infra.basetest.BRConfigTest):
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_NEON=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_PACKAGE_HOST_RUSTC=y
        LINGMO_PACKAGE_RIPGREP=y
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_CRYPTOGRAPHY=y
        LINGMO_BACKUP_SITE=""
        """

    def setUp(self):
        super(TestRustVendoring, self).setUp()

    def tearDown(self):
        self.show_msg("Cleaning up")
        if self.b and not self.keepbuilds:
            self.b.delete()

    def check_download(self, package):
        # store downloaded tarball inside the output dir so the test infra
        # cleans it up at the end
        dl_dir = os.path.join(self.builddir, "dl")
        # enforce we test the download
        if os.path.exists(dl_dir):
            shutil.rmtree(dl_dir)
        env = {"LINGMO_DL_DIR": dl_dir}
        self.b.build(["{}-dirclean".format(package),
                      "{}-legal-info".format(package)],
                     env)

    def test_run(self):
        self.check_download("ripgrep")
        self.check_download("python-cryptography")
