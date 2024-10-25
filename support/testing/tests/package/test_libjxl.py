import os

import infra.basetest


class TestLibJXL(infra.basetest.BRTest):
    # infra.basetest.BASIC_TOOLCHAIN_CONFIG is not used as it is armv5
    # and the image encoding would take too long (several minutes).
    # We also add GraphicsMagick to generate and compare images for
    # the test.
    config = \
        """
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.15.79"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_GZIP=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_PACKAGE_GRAPHICSMAGICK=y
        LINGMO_PACKAGE_LIBJXL=y
        """

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio.gz")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["console=ttyAMA0"],
                           options=["-M", "virt", "-cpu", "cortex-a57", "-m", "256M", "-initrd", img])
        self.emulator.login()

        ref = "/var/tmp/reference.ppm"
        jxl = "/var/tmp/encoded.jxl"
        dec = "/var/tmp/decoded.ppm"

        cmd = "gm convert IMAGE:LOGO {}".format(ref)
        self.assertRunOk(cmd)

        cmd = "cjxl {} {}".format(ref, jxl)
        self.assertRunOk(cmd, timeout=30)

        cmd = "djxl {} {}".format(jxl, dec)
        self.assertRunOk(cmd)

        cmd = "gm compare -metric mse -maximum-error 1e-3 {} {}".format(
            ref, dec)
        self.assertRunOk(cmd)
