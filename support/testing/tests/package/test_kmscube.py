import os

import infra.basetest


class TestKmsCube(infra.basetest.BRTest):
    config = \
        """
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.1.44"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{}"
        LINGMO_PACKAGE_KMSCUBE=y
        LINGMO_PACKAGE_LIBDRM=y
        LINGMO_PACKAGE_MESA3D=y
        LINGMO_PACKAGE_MESA3D_GALLIUM_DRIVER_SWRAST=y
        LINGMO_PACKAGE_MESA3D_LLVM=y
        LINGMO_PACKAGE_MESA3D_OPENGL_EGL=y
        LINGMO_PACKAGE_MESA3D_OPENGL_ES=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_GZIP=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """.format(
            infra.filepath("tests/package/test_kmscube/linux-vkms.fragment"))

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio.gz")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["console=ttyAMA0"],
                           options=["-M", "virt",
                                    "-cpu", "cortex-a57",
                                    "-smp", "4",
                                    "-m", "256M",
                                    "-initrd", img])
        self.emulator.login()

        cmd = "kmscube --vmode=640x480 --count=10"
        self.assertRunOk(cmd, timeout=30)
