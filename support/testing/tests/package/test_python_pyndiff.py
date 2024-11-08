import os
import infra.basetest


class TestPyNdiff(infra.basetest.BRTest):
    config = infra.basetest.BASIC_TOOLCHAIN_CONFIG + \
        """
        LINGMO_PACKAGE_NMAP=y
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_PYNDIFF=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv5",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()

        cmd = "nmap -F 127.0.0.1 -oX scanme-1.xml"
        self.assertRunOk(cmd)

        cmd = "nmap -F 127.0.0.1 -oX scanme-2.xml"
        self.assertRunOk(cmd)

        cmd = "pyndiff -f1 scanme-1.xml -f2 scanme-2.xml -t txt"
        self.assertRunOk(cmd)
