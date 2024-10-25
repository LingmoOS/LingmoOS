import os

import infra.basetest


class TestOpenJdk(infra.basetest.BRTest):
    br2_external = [infra.filepath("tests/package/br2-external/openjdk")]
    config = \
        """
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.10.34"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_GZIP=y
        LINGMO_PACKAGE_XORG7=y
        LINGMO_PACKAGE_OPENJDK=y
        LINGMO_PACKAGE_OPENJDK_HELLO_WORLD=y
        LINGMO_PACKAGE_OPENJDK_JNI_TEST=y
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

        cmd = "java -cp /usr/bin HelloWorld"
        output, exit_code = self.emulator.run(cmd, 120)
        print(output)
        self.assertEqual(exit_code, 0)
        self.assertEqual(output, ["Hello, World"])

        cmd = "java -cp /usr/bin JniTest"
        output, exit_code = self.emulator.run(cmd, 120)
        print(output)
        self.assertEqual(exit_code, 0)
