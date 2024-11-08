import os

import infra.basetest


class TestFileCapabilities(infra.basetest.BRTest):
    config = \
        """
        LINGMO_arm=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_ROOTFS_DEVICE_TABLE="system/device_table.txt {}"
        LINGMO_ROOTFS_DEVICE_TABLE_SUPPORTS_EXTENDED_ATTRIBUTES=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="4.19.310"
        LINGMO_LINUX_KERNEL_DEFCONFIG="vexpress"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{}"
        LINGMO_LINUX_KERNEL_DTS_SUPPORT=y
        LINGMO_LINUX_KERNEL_INTREE_DTS_NAME="vexpress-v2p-ca9"
        LINGMO_PACKAGE_LIBCAP=y
        LINGMO_PACKAGE_LIBCAP_TOOLS=y
        LINGMO_TARGET_ROOTFS_SQUASHFS=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """.format(infra.filepath("tests/core/device_table2.txt"),
                   infra.filepath("tests/core/squashfs-xattr-kernel.config"))

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.squashfs")
        infra.img_round_power2(img)

        self.emulator.boot(arch="armv7",
                           kernel=os.path.join(self.builddir, "images", "zImage"),
                           kernel_cmdline=["root=/dev/mmcblk0",
                                           "rootfstype=squashfs"],
                           options=["-drive", "file={},if=sd,format=raw".format(img),
                                    "-M", "vexpress-a9",
                                    "-dtb", os.path.join(self.builddir, "images", "vexpress-v2p-ca9.dtb")])
        self.emulator.login()

        cmd = "getcap -v /usr/sbin/getcap"
        output, _ = self.emulator.run(cmd)
        self.assertIn("cap_kill", output[0])
        self.assertIn("cap_sys_nice", output[0])
        self.assertIn("cap_sys_time", output[0])
        self.assertIn("=eip", output[0])
