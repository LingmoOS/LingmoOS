import os

import infra.basetest


class TestUsbUtils(infra.basetest.BRTest):
    # A specific configuration is needed for testing usbutils, to
    # enable USB 2.0 support in the Kernel.
    linux_fragment = \
        infra.filepath("tests/package/test_usbutils/linux-usbutils.fragment")
    config = \
        f"""
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.1.73"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{linux_fragment}"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_PACKAGE_EUDEV=y
        LINGMO_PACKAGE_USBUTILS=y
        LINGMO_ROOTFS_DEVICE_CREATION_DYNAMIC_EUDEV=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_GZIP=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio.gz")
        kern = os.path.join(self.builddir, "images", "Image")
        # We add a USB keyboard and mouse devices for the test.
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["console=ttyAMA0"],
                           options=["-M", "virt", "-cpu", "cortex-a57", "-m", "256M",
                                    "-initrd", img,
                                    "-device", "usb-ehci,id=ehci",
                                    "-device", "usb-kbd,bus=ehci.0",
                                    "-device", "usb-mouse,bus=ehci.0"])
        self.emulator.login()

        # Check the program can execute. We also check the version
        # string to make sure we have the usbutils version. The
        # BusyBox lsusb ignores arguments.
        output, exit_code = self.emulator.run("lsusb --version")
        self.assertEqual(exit_code, 0)
        self.assertTrue(output[0].startswith("lsusb (usbutils)"))

        # Test few simple and common invocations
        self.assertRunOk("lsusb")
        self.assertRunOk("lsusb --tree")
        self.assertRunOk("lsusb --verbose")
        # 1d6b:0002 is Linux Foundation 2.0 root hub
        # it should be present. lsusb return an error if no device
        # is found.
        self.assertRunOk("lsusb -d 1d6b:0002")
        # we emulate a USB keyboard and mouse, so usbhid-dump should find them
        self.assertRunOk("usbhid-dump")
