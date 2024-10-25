import os

import infra.basetest


class TestTraceCmd(infra.basetest.BRTest):
    # A specific configuration is needed for testing trace-cmd.
    # The function tracer need to be enabled in the Kernel.
    kern_fragment = \
        infra.filepath("tests/package/test_trace_cmd/linux-ftrace.fragment")
    config = \
        f"""
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.1.74"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{kern_fragment}"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_PACKAGE_TRACE_CMD=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_GZIP=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio.gz")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["console=ttyAMA0"],
                           options=["-M", "virt",
                                    "-cpu", "cortex-a57",
                                    "-m", "256M",
                                    "-initrd", img])
        self.emulator.login()

        # Record calls to kmalloc() from a simple command.
        self.assertRunOk("trace-cmd record -e kmalloc ls -l /")

        # Show information about the trace.dat file.
        self.assertRunOk("trace-cmd dump")

        # Generate a text report of the trace.
        self.assertRunOk("trace-cmd report > trace.txt")

        # Check we have occurrences of "kmalloc:" in the trace report.
        cmd = "grep -Fc kmalloc: trace.txt"
        output, exit_code = self.emulator.run(cmd)
        self.assertEqual(exit_code, 0)
        self.assertTrue(int(output[0]) > 0)
