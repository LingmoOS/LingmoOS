import os

import infra.basetest


class TestLxc(infra.basetest.BRTest):
    config = \
            """
            LINGMO_arm=y
            LINGMO_cortex_a9=y
            LINGMO_ARM_ENABLE_VFP=y
            LINGMO_TOOLCHAIN_EXTERNAL=y
            LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
            LINGMO_LINUX_KERNEL=y
            LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
            LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="5.15.38"
            LINGMO_LINUX_KERNEL_DEFCONFIG="vexpress"
            LINGMO_LINUX_KERNEL_DTS_SUPPORT=y
            LINGMO_LINUX_KERNEL_INTREE_DTS_NAME="vexpress-v2p-ca9"
            LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{}"
            LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
            LINGMO_INIT_SYSTEMD=y
            LINGMO_PACKAGE_LXC=y
            LINGMO_PACKAGE_TINI=y
            LINGMO_PACKAGE_IPERF3=y
            LINGMO_ROOTFS_OVERLAY="{}"
            LINGMO_TARGET_ROOTFS_CPIO=y
            """.format(
              infra.filepath("tests/package/test_lxc/lxc-kernel.config"),
              infra.filepath("tests/package/test_lxc/rootfs-overlay"))

    def run_ok(self, cmd):
        self.assertRunOk(cmd, 120)

    def wait_boot(self):
        # the complete boot with systemd takes more time than what the
        # default typically allows
        self.emulator.login(timeout=600)

    def setup_run_test_container(self):
        self.run_ok("lxc-create -n lxc_iperf3 -t none -f /usr/share/lxc/config/minimal-iperf3.conf")
        self.run_ok("lxc-start -l trace -n lxc_iperf3 -o /tmp/lxc.log -L /tmp/lxc.console.log")
        # need to wait for the container to be fully started
        self.run_ok("sleep 2")
        self.run_ok("iperf3 -c 192.168.1.2 -t 2")
        # if the test fails, just cat /tmp/*.log

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        kernel_file = os.path.join(self.builddir, "images", "zImage")
        dtb_file = os.path.join(self.builddir, "images", "vexpress-v2p-ca9.dtb")
        self.emulator.boot(arch="armv7", kernel=kernel_file,
                           kernel_cmdline=[
                                        "console=ttyAMA0,115200"],
                           options=["-initrd", cpio_file,
                                    "-dtb", dtb_file,
                                    "-M", "vexpress-a9"])
        self.wait_boot()
        self.setup_run_test_container()
