import os

import infra.basetest


class TestGlslsandboxPlayer(infra.basetest.BRTest):
    config = \
        """
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.1.32"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{}"
        LINGMO_PACKAGE_LIBDRM=y
        LINGMO_PACKAGE_MESA3D=y
        LINGMO_PACKAGE_MESA3D_GALLIUM_DRIVER_SWRAST=y
        LINGMO_PACKAGE_MESA3D_LLVM=y
        LINGMO_PACKAGE_MESA3D_OPENGL_EGL=y
        LINGMO_PACKAGE_MESA3D_OPENGL_ES=y
        LINGMO_PACKAGE_GLSLSANDBOX_PLAYER=y
        LINGMO_PACKAGE_GLSLSANDBOX_PLAYER_KMS=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_GZIP=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """.format(
            infra.filepath("tests/package/test_glslsandbox_player/linux-vkms.fragment"))

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio.gz")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["console=ttyAMA0"],
                           options=["-M", "virt", "-cpu", "cortex-a57", "-m", "256M", "-initrd", img])
        self.emulator.login()

        # We force a small resolution in order to keep a relatively
        # fast software rendering
        cmd = "GSP_DRM_MODE=640x480 "
        # We run 3 frames of a reduced resolution of SimpleMandel
        cmd += "glslsandbox-player -S SimpleMandel -w0 -f3 -R8 -N -vv -r1 -D"
        self.assertRunOk(cmd, timeout=30)

        # Since we render 3 frames and request a dump of the last one,
        # a ppm image file is expected to be present
        self.assertRunOk("test -s SimpleMandel-00002.ppm")
