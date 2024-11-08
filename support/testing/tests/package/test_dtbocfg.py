import os

import infra.basetest


class TestDtbocfg(infra.basetest.BRTest):
    config = infra.basetest.BASIC_TOOLCHAIN_CONFIG + \
        """
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.10.202"
        LINGMO_LINUX_KERNEL_USE_DEFCONFIG=y
        LINGMO_LINUX_KERNEL_DEFCONFIG="vexpress"
        LINGMO_PACKAGE_DTBOCFG=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        kernel = os.path.join(self.builddir, "images", "zImage")
        kernel_cmdline = ["console=ttyAMA0"]
        dtb = infra.download(self.downloaddir, "vexpress-v2p-ca9.dtb")
        options = ["-M", "vexpress-a9", "-dtb", dtb, "-initrd", img]

        self.emulator.boot(arch="armv7", kernel=kernel,
                           kernel_cmdline=kernel_cmdline,
                           options=options)
        self.emulator.login()
        self.assertRunOk("modprobe dtbocfg.ko")
        self.assertRunOk("mkdir /tmp/config && mount -t configfs none /tmp/config")

        # Check that overlays directory is present.
        # From dtbocfg site (https://github.com/ikwzm/dtbocfg):
        #   If /config/device-tree/overlays is created, it is ready to use
        #   dtbocfg
        self.assertRunOk("ls /tmp/config/device-tree/overlays/")
