import os

import infra.basetest


class TestPerftest(infra.basetest.BRTest):

    config = \
        """
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.1.33"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{}"
        LINGMO_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_TARGET_ROOTFS_CPIO_GZIP=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        LINGMO_PACKAGE_IPROUTE2=y
        LINGMO_PACKAGE_LIBMNL=y
        LINGMO_PACKAGE_RDMA_CORE=y
        LINGMO_PACKAGE_PERFTEST=y
        """.format(
            infra.filepath("tests/package/test_rdma_core/linux-rdma.fragment")
        )

    def test_run(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio.gz")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["console=ttyAMA0"],
                           options=["-M", "virt", "-cpu", "cortex-a57", "-m", "512M", "-initrd", img])
        self.emulator.login()

        # Add the rxe0 interface
        self.assertRunOk("ip link set dev eth0 up")
        self.assertRunOk("rdma link add rxe0 type rxe netdev eth0")

        # start a server
        self.assertRunOk("ib_read_bw > /dev/null 2>&1 &")

        # start a client
        self.assertRunOk("sleep 1 && ib_read_bw 127.0.0.1")
