import os

import infra.basetest


class TestEthtool(infra.basetest.BRTest):
    # This ethtool test uses an emulated Intel e1000e adapted (which
    # is well supported by Qemu, the Kernel and ethtool). We are not
    # using the usual virtio_net because it's not supporting some
    # ethtool features like adapter testing. For that reason, we need
    # to compile a Kernel.
    kernel_fragment = \
        infra.filepath("tests/package/test_ethtool/linux-e1000e.fragment")
    config = \
        f"""
        LINGMO_aarch64=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        LINGMO_LINUX_KERNEL=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION=y
        LINGMO_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.6.27"
        LINGMO_LINUX_KERNEL_USE_CUSTOM_CONFIG=y
        LINGMO_LINUX_KERNEL_CUSTOM_CONFIG_FILE="board/qemu/aarch64-virt/linux.config"
        LINGMO_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="{kernel_fragment}"
        LINGMO_PACKAGE_ETHTOOL=y
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_ROOTFS_EXT2_4=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        drive = os.path.join(self.builddir, "images", "rootfs.ext4")
        kern = os.path.join(self.builddir, "images", "Image")
        self.emulator.boot(arch="aarch64",
                           kernel=kern,
                           kernel_cmdline=["root=/dev/vda console=ttyAMA0"],
                           options=["-M", "virt",
                                    "-cpu", "cortex-a57",
                                    "-m", "256M",
                                    "-netdev", "user,id=test_net",
                                    "-net", "nic,model=e1000e,netdev=test_net",
                                    "-drive", f"file={drive},if=virtio,format=raw"])
        self.emulator.login()

        # We check the program can run.
        self.assertRunOk("ethtool --version")

        # We check a simple query runs correctly.
        self.assertRunOk("ethtool eth0")

        # We query the driver name and check it's the expected e1000e.
        out, ret = self.emulator.run("ethtool -i eth0")
        self.assertEqual(ret, 0)
        self.assertEqual(out[0], "driver: e1000e")

        # We request an adapter online self test.
        self.assertRunOk("ethtool -t eth0 online", timeout=30)

        # We query offload parameters and check TSO are enabled (this
        # is the driver default).
        out, ret = self.emulator.run("ethtool -k eth0")
        self.assertEqual(ret, 0)
        self.assertIn("tcp-segmentation-offload: on", out)

        # We request to disable TSO.
        self.assertRunOk("ethtool -K eth0 tcp-segmentation-offload off")

        # We check again. TSO should now be disabled.
        out, ret = self.emulator.run("ethtool -k eth0")
        self.assertEqual(ret, 0)
        self.assertIn("tcp-segmentation-offload: off", out)
