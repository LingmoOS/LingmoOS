import os

import infra.basetest

BASIC_CONFIG = \
    """
    LINGMO_x86_pentium4=y
    LINGMO_TOOLCHAIN_EXTERNAL=y
    LINGMO_TOOLCHAIN_EXTERNAL_CUSTOM=y
    LINGMO_TOOLCHAIN_EXTERNAL_DOWNLOAD=y
    LINGMO_TOOLCHAIN_EXTERNAL_URL="http://autobuild.buildroot.org/toolchains/tarballs/br-i386-pentium4-full-2017.05-1078-g95b1dae.tar.bz2"
    LINGMO_TOOLCHAIN_EXTERNAL_GCC_6=y
    LINGMO_TOOLCHAIN_EXTERNAL_HEADERS_3_2=y
    LINGMO_TOOLCHAIN_EXTERNAL_LOCALE=y
    # LINGMO_TOOLCHAIN_EXTERNAL_HAS_THREADS_DEBUG is not set
    LINGMO_TOOLCHAIN_EXTERNAL_CXX=y
    LINGMO_TARGET_GENERIC_GETTY_PORT="ttyS0"
    LINGMO_TARGET_GENERIC_GETTY_BAUDRATE_115200=y
    LINGMO_LINUX_KERNEL=y
    LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
    LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="4.19.204"
    LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
    LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="{}"
    # LINGMO_TARGET_ROOTFS_TAR is not set
    """.format(infra.filepath("conf/minimal-x86-qemu-kernel.config"))


def test_mount_internal_external(emulator, builddir, internal=True, efi=False):
    img = os.path.join(builddir, "images", "rootfs.iso9660")
    if efi:
        efi_img = os.path.join(builddir, "images", "OVMF.fd")
        # In QEMU v5.1.0 up to v7.2.0, the CPU hotplug register block misbehaves.
        # EDK2 hang if the bug is detected in Qemu after printing errors to IO port 0x402
        # (requires LINGMO_TARGET_EDK2_OVMF_DEBUG_ON_SERIAL to see them)
        # The Docker image used by the Buildroot gitlab-ci uses Qemu 5.2.0, the workaround
        # can be removed as soon as the Docker image is updated to provided Qemu >= 8.0.0.
        # This workaround is needed only when efi=True since it imply EDK2 is used.
        # https://github.com/tianocore/edk2/commit/bf5678b5802685e07583e3c7ec56d883cbdd5da3
        # http://lists.busybox.net/pipermail/buildroot/2023-July/670825.html
        qemu_fw_cfg = "name=opt/org.tianocore/X-Cpuhp-Bugcheck-Override,string=yes"
        emulator.boot(arch="i386", options=["-cdrom", img, "-bios", efi_img, "-fw_cfg", qemu_fw_cfg])
    else:
        emulator.boot(arch="i386", options=["-cdrom", img])
    emulator.login()

    if internal:
        cmd = "mount | grep 'rootfs on / type rootfs'"
    else:
        cmd = "mount | grep '/dev/root on / type iso9660'"

    _, exit_code = emulator.run(cmd)
    return exit_code


def test_touch_file(emulator):
    _, exit_code = emulator.run("touch test")
    return exit_code

#
# Grub 2


class TestIso9660Grub2External(infra.basetest.BRTest):
    config = BASIC_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_ISO9660=y
        # LINGMO_TARGET_ROOTFS_ISO9660_INITRD is not set
        LINGMO_TARGET_GRUB2=y
        LINGMO_TARGET_GRUB2_BOOT_PARTITION="cd"
        LINGMO_TARGET_GRUB2_BUILTIN_MODULES_PC="boot linux ext2 fat part_msdos part_gpt normal biosdisk iso9660"
        LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU="{}"
        """.format(infra.filepath("conf/grub2.cfg"))

    def test_run(self):
        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=False)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 1)


class TestIso9660Grub2ExternalCompress(infra.basetest.BRTest):
    config = BASIC_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_ISO9660=y
        # LINGMO_TARGET_ROOTFS_ISO9660_INITRD is not set
        LINGMO_TARGET_ROOTFS_ISO9660_TRANSPARENT_COMPRESSION=y
        LINGMO_TARGET_GRUB2=y
        LINGMO_TARGET_GRUB2_BOOT_PARTITION="cd"
        LINGMO_TARGET_GRUB2_BUILTIN_MODULES_PC="boot linux ext2 fat part_msdos part_gpt normal biosdisk iso9660"
        LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU="{}"
        """.format(infra.filepath("conf/grub2.cfg"))

    def test_run(self):
        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=False)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 1)


class TestIso9660Grub2Internal(infra.basetest.BRTest):
    config = BASIC_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_ISO9660=y
        LINGMO_TARGET_ROOTFS_ISO9660_INITRD=y
        LINGMO_TARGET_GRUB2=y
        LINGMO_TARGET_GRUB2_BOOT_PARTITION="cd"
        LINGMO_TARGET_GRUB2_BUILTIN_MODULES_PC="boot linux ext2 fat part_msdos part_gpt normal biosdisk iso9660"
        LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU="{}"
        """.format(infra.filepath("conf/grub2.cfg"))

    def test_run(self):
        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=True)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 0)


class TestIso9660Grub2EFI(infra.basetest.BRTest):
    config = BASIC_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_ISO9660=y
        LINGMO_TARGET_ROOTFS_ISO9660_INITRD=y
        LINGMO_TARGET_GRUB2=y
        LINGMO_TARGET_GRUB2_I386_EFI=y
        LINGMO_TARGET_GRUB2_BUILTIN_MODULES_EFI="boot linux ext2 fat part_msdos part_gpt normal iso9660"
        LINGMO_TARGET_GRUB2_BUILTIN_CONFIG_EFI="{}"
        LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU="{}"
        LINGMO_TARGET_EDK2=y
        """.format(infra.filepath("conf/grub2-efi.cfg"),
                   infra.filepath("conf/grub2.cfg"))

    def test_run(self):
        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=True,
                                                 efi=True)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 0)


class TestIso9660Grub2Hybrid(infra.basetest.BRTest):
    config = BASIC_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_ISO9660=y
        LINGMO_TARGET_ROOTFS_ISO9660_INITRD=y
        LINGMO_TARGET_GRUB2=y
        LINGMO_TARGET_GRUB2_I386_PC=y
        LINGMO_TARGET_GRUB2_I386_EFI=y
        LINGMO_TARGET_GRUB2_BOOT_PARTITION="cd"
        LINGMO_TARGET_GRUB2_BUILTIN_MODULES_PC="boot linux ext2 fat squash4 part_msdos part_gpt normal iso9660 biosdisk"
        LINGMO_TARGET_GRUB2_BUILTIN_CONFIG_PC=""
        LINGMO_TARGET_GRUB2_BUILTIN_MODULES_EFI="boot linux ext2 fat squash4 part_msdos part_gpt normal iso9660 efi_gop"
        LINGMO_TARGET_GRUB2_BUILTIN_CONFIG_EFI="{}"
        LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU="{}"
        LINGMO_TARGET_EDK2=y
        """.format(infra.filepath("conf/grub2-efi.cfg"),
                   infra.filepath("conf/grub2.cfg"))

    def test_run(self):
        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=True,
                                                 efi=False)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 0)

        self.emulator.stop()

        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=True,
                                                 efi=True)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 0)


#
# Syslinux


class TestIso9660SyslinuxExternal(infra.basetest.BRTest):
    config = BASIC_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_ISO9660=y
        # LINGMO_TARGET_ROOTFS_ISO9660_INITRD is not set
        LINGMO_TARGET_ROOTFS_ISO9660_HYBRID=y
        LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU="{}"
        LINGMO_TARGET_SYSLINUX=y
        """.format(infra.filepath("conf/isolinux.cfg"))

    def test_run(self):
        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=False)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 1)


class TestIso9660SyslinuxExternalCompress(infra.basetest.BRTest):
    config = BASIC_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_ISO9660=y
        # LINGMO_TARGET_ROOTFS_ISO9660_INITRD is not set
        LINGMO_TARGET_ROOTFS_ISO9660_TRANSPARENT_COMPRESSION=y
        LINGMO_TARGET_ROOTFS_ISO9660_HYBRID=y
        LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU="{}"
        LINGMO_TARGET_SYSLINUX=y
        """.format(infra.filepath("conf/isolinux.cfg"))

    def test_run(self):
        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=False)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 1)


class TestIso9660SyslinuxInternal(infra.basetest.BRTest):
    config = BASIC_CONFIG + \
        """
        LINGMO_TARGET_ROOTFS_ISO9660=y
        LINGMO_TARGET_ROOTFS_ISO9660_INITRD=y
        LINGMO_TARGET_ROOTFS_ISO9660_HYBRID=y
        LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU="{}"
        LINGMO_TARGET_SYSLINUX=y
        """.format(infra.filepath("conf/isolinux.cfg"))

    def test_run(self):
        exit_code = test_mount_internal_external(self.emulator,
                                                 self.builddir, internal=True)
        self.assertEqual(exit_code, 0)

        exit_code = test_touch_file(self.emulator)
        self.assertEqual(exit_code, 0)
