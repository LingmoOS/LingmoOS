import os
import re

import infra.basetest


class TestAarch64Pages64kBase(infra.basetest.BRTest):
    __test__ = False
    config = \
        """
        LINGMO_aarch64=y
        LINGMO_ARM64_PAGE_SIZE_64K=y
        LINGMO_PACKAGE_HOST_LINUX_HEADERS_CUSTOM_5_15=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.15.18"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_GZIP=y
        """

    def login(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio.gz")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["console=ttyAMA0"],
                           options=["-M", "virt", "-cpu", "cortex-a57", "-m", "512M", "-initrd", img])
        self.emulator.login()

    def test_run(self):
        self.login()

        cmd = "dmesg | grep 'Dentry cache'"
        output, exit_code = self.emulator.run(cmd, 120)
        r = re.match(r".*Dentry cache hash table entries: [0-9]* \(order: ([0-9]*), ([0-9]*) bytes.*", output[0])
        order = int(r.group(1))
        size = int(r.group(2))
        self.assertEqual(2 ** order * 64 * 1024, size)


class TestAarch64Pages64kGlibc(TestAarch64Pages64kBase):
    __test__ = True
    config = TestAarch64Pages64kBase.config + \
        """
        LINGMO_TOOLCHAIN_BUILDROOT_GLIBC=y
        """


class TestAarch64Pages64kuClibc(TestAarch64Pages64kBase):
    __test__ = True
    config = TestAarch64Pages64kBase.config + \
        """
        LINGMO_TOOLCHAIN_BUILDROOT_UCLIBC=y
        """


class TestAarch64Pages64kMusl(TestAarch64Pages64kBase):
    __test__ = True
    config = TestAarch64Pages64kBase.config + \
        """
        LINGMO_TOOLCHAIN_BUILDROOT_MUSL=y
        """
