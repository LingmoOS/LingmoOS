import os

import infra.basetest


class TestMsrTools(infra.basetest.BRTest):
    config = \
        """
        LINGMO_x86_64=y
        LINGMO_x86_corei7=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.15.55"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/x86_64/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{}"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_LIBELF=y
        LINGMO_PACKAGE_MSR_TOOLS=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """.format(
              infra.filepath("tests/package/test_msr_tools/linux.config"))

    def test_run(self):
        kernel = os.path.join(self.builddir, "images", "bzImage")
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(
            arch="x86_64",
            kernel=kernel, kernel_cmdline=["console=ttyS0"],
            options=["-cpu", "Nehalem", "-m", "320", "-initrd", cpio_file]
        )
        self.emulator.login()

        # CPU ID.
        cmd = "cpuid"
        self.assertRunOk(cmd)

        # Write MSR.
        # We write to TSC_AUX.
        cmd = "wrmsr 0xc0000103 0x1234567812345678"
        self.assertRunOk(cmd)

        # Read MSR.
        # We read back the TSC_AUX and we verify that we read back the correct
        # value.
        cmd = "rdmsr 0xc0000103"
        output, exit_code = self.emulator.run(cmd)
        self.assertEqual(exit_code, 0)
        self.assertEqual(output[0], "1234567812345678")
