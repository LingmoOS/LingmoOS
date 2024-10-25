import os
import shutil
import infra.basetest


class TestOci(infra.basetest.BRTest):
    config = \
        """
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.10.61"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_PACKAGE_CGROUPFS_MOUNT=y
        LINGMO_PACKAGE_CONTAINERD=y
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_ROOTFS_EXT2_SIZE="600M"
        LINGMO_TARGET_ROOTFS_OCI=y
        LINGMO_TARGET_ROOTFS_OCI_ENTRYPOINT="df"
        LINGMO_TARGET_ROOTFS_OCI_CMD="-h"
        LINGMO_TARGET_ROOTFS_OCI_ARCHIVE=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def login(self):
        rootfs = os.path.join(self.builddir, "images", "rootfs.ext2")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["root=/dev/vda", "console=ttyAMA0"],
                           options=["-M", "virt",
                                    "-cpu", "cortex-a57",
                                    "-m", "512M",
                                    "-drive", "file={},format=raw,if=virtio".format(rootfs)])
        self.emulator.login()

    def place_test_oci(self):
        shutil.copy(os.path.join(self.builddir, 'images', 'rootfs-oci-latest-arm64-linux.oci-image.tar'),
                    os.path.join(self.builddir, 'target', 'oci.tar'))
        # rebuild to make sure oci.tar ends up in rootfs.ext2
        self.b.build()

    def test_run(self):
        self.place_test_oci()
        self.login()

        cmd = "containerd &"
        self.assertRunOk(cmd)

        cmd = "ctr image import --base-name buildroot-test /oci.tar"
        self.assertRunOk(cmd, timeout=120)

        cmd = "ctr run --rm --tty buildroot-test:latest v1"
        self.assertRunOk(cmd, timeout=120)
