import os
import infra.basetest
import subprocess

CHECK_FS_CMD = "mount | grep 'rootfs on / type rootfs'"


def boot_img(emulator, builddir):
    img = os.path.join(builddir, "images", "rootfs.cpio")
    emulator.boot(arch="armv7",
                  kernel="builtin",
                  options=["-initrd", "{}".format(img)])
    emulator.login()
    _, exit_code = emulator.run(CHECK_FS_CMD)
    return exit_code


class TestCpioFull(infra.basetest.BRTest):
    config = infra.basetest.BASIC_TOOLCHAIN_CONFIG + \
        """
        LINGMO_INIT_BUSYBOX=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):

        exit_code = boot_img(self.emulator,
                             self.builddir)
        self.assertEqual(exit_code, 0)


class TestCpioDracutBase(infra.basetest.BRTest):
    config = \
        """
        LINGMO_arm=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_INIT_BUSYBOX=y
        LINGMO_PACKAGE_CRAMFS=y
        LINGMO_PACKAGE_PV=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_DRACUT=y
        LINGMO_TARGET_ROOTFS_CPIO_DRACUT_MODULES="{}"
        LINGMO_TARGET_ROOTFS_CPIO_DRACUT_CONF_FILES="{}"
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """.format("support/testing/tests/fs/test_cpio/modules",
                   " ".join(["fs/cpio/dracut.conf",
                             "support/testing/tests/fs/test_cpio/dracut-cramfs.conf"]))

    def check_dracut(self):
        out = subprocess.check_output(["cpio", "--list"],
                                      stdin=open(os.path.join(self.builddir, "images/rootfs.cpio")),
                                      stderr=open(os.devnull, "w"),
                                      cwd=self.builddir,
                                      env={"LANG": "C"},
                                      universal_newlines=True)
        # pv should *not* be included in cpio image
        self.assertEqual(out.find("bin/pv"), -1)
        # libz should be, because of cramfs
        self.assertNotEqual(out.find("usr/bin/mkcramfs"), -1)
        self.assertNotEqual(out.find("usr/bin/cramfsck"), -1)
        self.assertNotEqual(out.find("usr/lib/libz.so"), -1)

        exit_code = boot_img(self.emulator,
                             self.builddir)
        self.assertEqual(exit_code, 0)


class TestCpioDracutUclibc(TestCpioDracutBase):
    config = TestCpioDracutBase.config + \
        """
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_ARMV5_EABI_UCLIBC_STABLE=y
        """

    def test_run(self):
        self.check_dracut()


class TestCpioDracutGlibc(TestCpioDracutBase):
    config = TestCpioDracutBase.config + \
        """
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_ARMV5_EABI_GLIBC_STABLE=y
        """

    def test_run(self):
        self.check_dracut()


class TestCpioDracutMusl(TestCpioDracutBase):
    config = TestCpioDracutBase.config + \
        """
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_ARMV5_EABI_MUSL_STABLE=y
        """

    def test_run(self):
        self.check_dracut()


class TestCpioDracutUclibcMergedUsr(TestCpioDracutBase):
    config = TestCpioDracutBase.config + \
        """
        LINGMO_ROOTFS_MERGED_USR=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_ARMV5_EABI_UCLIBC_STABLE=y
        """

    def test_run(self):
        self.check_dracut()


class TestCpioDracutGlibcMergedUsr(TestCpioDracutBase):
    config = TestCpioDracutBase.config + \
        """
        LINGMO_ROOTFS_MERGED_USR=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_ARMV5_EABI_GLIBC_STABLE=y
        """

    def test_run(self):
        self.check_dracut()


class TestCpioDracutMuslMergedUsr(TestCpioDracutBase):
    config = TestCpioDracutBase.config + \
        """
        LINGMO_ROOTFS_MERGED_USR=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_ARMV5_EABI_MUSL_STABLE=y
        """

    def test_run(self):
        self.check_dracut()
