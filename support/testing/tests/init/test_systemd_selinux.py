import os

import infra.basetest


class TestSELinuxSystemd(infra.basetest.BRTest):
    config = \
        """
        LINGMO_x86_64=y
        LINGMO_x86_corei7=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_INIT_SYSTEMD=y
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.1.26"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/x86_64/linux.config"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_LIBELF=y
        LINGMO_PACKAGE_LIBSELINUX=y
        LINGMO_PACKAGE_REFPOLICY=y
        """

    def wait_boot(self):
        # The complete boot with systemd takes more time than what the
        # default typically allows
        self.emulator.login(timeout=600)

    def run_tests(self, fstype):
        kernel = os.path.join(self.builddir, "images", "bzImage")
        rootfs = os.path.join(self.builddir, "images", "rootfs.{}".format(fstype))

        self.emulator.boot(arch="x86_64", kernel=kernel,
                           kernel_cmdline=["root=/dev/vda", "rootfstype={}".format(fstype),
                                           "console=ttyS0", "security=selinux"],
                           options=["-cpu", "Nehalem",
                                    "-drive", "file={},if=virtio,format=raw".format(rootfs)])
        self.wait_boot()

        # Test the reported SELinux mode.
        out, ret = self.emulator.run("getenforce")
        self.assertEqual(ret, 0)
        self.assertEqual(out[0], "Permissive")

        # Check the extended arguments are correctly set.
        out, ret = self.emulator.run("ls -dZ /")
        self.assertEqual(ret, 0)
        self.assertEqual(out[0].split()[0], "system_u:object_r:root_t")

        # Check init's attributes.
        out, ret = self.emulator.run("cat /proc/1/attr/current")
        self.assertEqual(ret, 0)
        self.assertEqual(out[0], "system_u:system_r:init_t\0")


class TestSELinuxSystemdExt4(TestSELinuxSystemd):
    config = TestSELinuxSystemd.config + \
        """
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_ROOTFS_EXT2_4=y
        LINGMO_TARGET_ROOTFS_EXT2_SIZE="100M"
        """

    def test_run(self):
        self.run_tests("ext4")


class TestSELinuxSystemdSquashfs(TestSELinuxSystemd):
    config = TestSELinuxSystemd.config + \
        """
        LINGMO_TARGET_ROOTFS_SQUASHFS=y
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{}"
        """.format(
            infra.filepath("tests/init/test_systemd_selinux/linux-squashfs.fragment"),
        )

    def test_run(self):
        self.run_tests("squashfs")
