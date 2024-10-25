import os
import infra.basetest


class TestFlutter(infra.basetest.BRTest):
    config = f"""
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_ROOTFS_OVERLAY="{infra.filepath("tests/package/test_flutter/overlay")}"
        LINGMO_PER_PACKAGE_DIRECTORIES=y
        LINGMO_INIT_SYSTEMD=y
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.1.54"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_NEEDS_HOST_LIBELF=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{infra.filepath("tests/package/test_flutter/linux-vkms.fragment")}"
        LINGMO_PACKAGE_LIBDRM=y
        LINGMO_PACKAGE_MESA3D=y
        LINGMO_PACKAGE_MESA3D_GALLIUM_DRIVER_SWRAST=y
        LINGMO_PACKAGE_MESA3D_GALLIUM_DRIVER_VIRGL=y
        LINGMO_PACKAGE_MESA3D_OPENGL_ES=y
        LINGMO_PACKAGE_FLUTTER_PI=y
        LINGMO_PACKAGE_FLUTTER_PI_RAW_KEYBOARD_PLUGIN=y
        LINGMO_PACKAGE_FLUTTER_PI_TEXT_INPUT_PLUGIN=y
        LINGMO_PACKAGE_FLUTTER_GALLERY=y
        LINGMO_PACKAGE_FLUTTER_ENGINE=y
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_ROOTFS_EXT2_4=y
        LINGMO_TARGET_ROOTFS_EXT2_SIZE="512M"
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.ext2")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(
            arch="aarch64",
            kernel=kern,
            kernel_cmdline=["root=/dev/vda console=ttyAMA0"],
            options=["-M", "virt",
                     "-cpu", "cortex-a57",
                     "-m", "512M",
                     "-smp", "4",
                     "-vga", "std",
                     "-vnc", "none",
                     "-drive", f"file={img},if=virtio,format=raw"])
        self.emulator.login()
        cmd = "systemctl is-active flutter-gallery"
        output, exit_code = self.emulator.run(cmd, 10)
        self.assertEqual(exit_code, 0)
        self.assertEqual(output[0], "active")
