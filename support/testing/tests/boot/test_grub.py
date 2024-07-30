import os

import infra.basetest


class TestGrubi386BIOS(infra.basetest.BRTest):
    config = \
        """
        LINGMO_x86_core2=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_X86_CORE2_UCLIBC_STABLE=y
        LINGMO_ROOTFS_POST_BUILD_SCRIPT="board/pc/post-build.sh {}"
        LINGMO_ROOTFS_POST_IMAGE_SCRIPT="support/scripts/genimage.sh"
        LINGMO_ROOTFS_POST_SCRIPT_ARGS="-c board/pc/genimage-bios.cfg"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="4.19.204"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/pc/linux.config"
        LINGMO_LINUX_KERNEL_INSTALL_TARGET=y
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_GRUB2=y
        LINGMO_TARGET_GRUB2_I386_PC=y
        LINGMO_TARGET_GRUB2_INSTALL_TOOLS=y
        LINGMO_PACKAGE_HOST_GENIMAGE=y
        """.format(infra.filepath("tests/boot/test_grub/post-build.sh"))

    def test_run(self):
        hda = os.path.join(self.builddir, "images", "disk.img")
        self.emulator.boot(arch="i386", options=["-hda", hda])
        self.emulator.login()


class TestGrubi386EFI(infra.basetest.BRTest):
    config = \
        """
        LINGMO_x86_core2=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_X86_CORE2_UCLIBC_STABLE=y
        LINGMO_ROOTFS_POST_BUILD_SCRIPT="board/pc/post-build.sh {}"
        LINGMO_ROOTFS_POST_IMAGE_SCRIPT="board/pc/post-image-efi.sh"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="4.19.204"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/pc/linux.config"
        LINGMO_LINUX_KERNEL_INSTALL_TARGET=y
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_PACKAGE_EFIVAR=y
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_EDK2=y
        LINGMO_TARGET_GRUB2=y
        LINGMO_TARGET_GRUB2_I386_EFI=y
        LINGMO_TARGET_GRUB2_INSTALL_TOOLS=y
        LINGMO_PACKAGE_HOST_GENIMAGE=y
        LINGMO_PACKAGE_HOST_MTOOLS=y
        LINGMO_PACKAGE_HOST_DOSFSTOOLS=y
        """.format(infra.filepath("tests/boot/test_grub/post-build.sh"))

    def test_run(self):
        hda = os.path.join(self.builddir, "images", "disk.img")
        bios = os.path.join(self.builddir, "images", "OVMF.fd")
        # In QEMU v5.1.0 up to v7.2.0, the CPU hotplug register block misbehaves.
        # EDK2 hang if the bug is detected in Qemu after printing errors to IO port 0x402
        # (requires LINGMO_TARGET_EDK2_OVMF_DEBUG_ON_SERIAL to see them)
        # The Docker image used by the Buildroot gitlab-ci uses Qemu 5.2.0, the workaround
        # can be removed as soon as the Docker image is updated to provided Qemu >= 8.0.0.
        # https://github.com/tianocore/edk2/commit/bf5678b5802685e07583e3c7ec56d883cbdd5da3
        # http://lists.busybox.net/pipermail/buildroot/2023-July/670825.html
        qemu_fw_cfg = "name=opt/org.tianocore/X-Cpuhp-Bugcheck-Override,string=yes"
        self.emulator.boot(arch="i386", options=["-bios", bios, "-hda", hda, "-fw_cfg", qemu_fw_cfg])
        self.emulator.login()

        cmd = "modprobe efivarfs"
        self.assertRunOk(cmd)

        cmd = "mount -t efivarfs none /sys/firmware/efi/efivars"
        self.assertRunOk(cmd)

        cmd = "efivar -l"
        self.assertRunOk(cmd)


class TestGrubX8664EFI(infra.basetest.BRTest):
    config = \
        """
        LINGMO_x86_64=y
        LINGMO_x86_corei7=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN_X86_64_UCLIBC_STABLE=y
        LINGMO_ROOTFS_POST_BUILD_SCRIPT="board/pc/post-build.sh {}"
        LINGMO_ROOTFS_POST_IMAGE_SCRIPT="board/pc/post-image-efi.sh"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="4.19.204"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/pc/linux.config"
        LINGMO_LINUX_KERNEL_INSTALL_TARGET=y
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_PACKAGE_EFIVAR=y
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_EDK2=y
        LINGMO_TARGET_GRUB2=y
        LINGMO_TARGET_GRUB2_X86_64_EFI=y
        LINGMO_TARGET_GRUB2_INSTALL_TOOLS=y
        LINGMO_PACKAGE_HOST_GENIMAGE=y
        LINGMO_PACKAGE_HOST_MTOOLS=y
        LINGMO_PACKAGE_HOST_DOSFSTOOLS=y
        """.format(infra.filepath("tests/boot/test_grub/post-build.sh"))

    def test_run(self):
        hda = os.path.join(self.builddir, "images", "disk.img")
        bios = os.path.join(self.builddir, "images", "OVMF.fd")
        # In QEMU v5.1.0 up to v7.2.0, the CPU hotplug register block misbehaves.
        # EDK2 hang if the bug is detected in Qemu after printing errors to IO port 0x402
        # (requires LINGMO_TARGET_EDK2_OVMF_DEBUG_ON_SERIAL to see them)
        # The Docker image used by the Buildroot gitlab-ci uses Qemu 5.2.0, the workaround
        # can be removed as soon as the Docker image is updated to provided Qemu >= 8.0.0.
        # https://github.com/tianocore/edk2/commit/bf5678b5802685e07583e3c7ec56d883cbdd5da3
        # http://lists.busybox.net/pipermail/buildroot/2023-July/670825.html
        qemu_fw_cfg = "name=opt/org.tianocore/X-Cpuhp-Bugcheck-Override,string=yes"
        self.emulator.boot(arch="x86_64", options=["-bios", bios, "-cpu", "Nehalem", "-hda", hda, "-fw_cfg", qemu_fw_cfg])
        self.emulator.login()

        cmd = "modprobe efivarfs"
        self.assertRunOk(cmd)

        cmd = "mount -t efivarfs none /sys/firmware/efi/efivars"
        self.assertRunOk(cmd)

        cmd = "efivar -l"
        self.assertRunOk(cmd)


class TestGrubAArch64EFI(infra.basetest.BRTest):
    config = \
        """
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_ROOTFS_POST_IMAGE_SCRIPT="{post_image}"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.15.18"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{linux_fragment}"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_PACKAGE_EFIVAR=y
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_EDK2=y
        LINGMO_TARGET_GRUB2=y
        LINGMO_PACKAGE_HOST_GENIMAGE=y
        LINGMO_PACKAGE_HOST_MTOOLS=y
        LINGMO_PACKAGE_HOST_DOSFSTOOLS=y
        """.format(post_image=infra.filepath("tests/boot/test_grub/post-image-aarch64-efi.sh"),
                   linux_fragment=infra.filepath("tests/boot/test_grub/linux-aarch64-efi.config"))

    def test_run(self):
        hda = os.path.join(self.builddir, "images", "disk.img")
        bios = os.path.join(self.builddir, "images", "QEMU_EFI.fd")
        self.emulator.boot(arch="aarch64", options=["-M", "virt", "-cpu", "cortex-a53", "-bios", bios, "-hda", hda])
        self.emulator.login()

        cmd = "modprobe efivarfs"
        self.assertRunOk(cmd)

        cmd = "mount -t efivarfs none /sys/firmware/efi/efivars"
        self.assertRunOk(cmd)

        cmd = "efivar -l"
        self.assertRunOk(cmd)
