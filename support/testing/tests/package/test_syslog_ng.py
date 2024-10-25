import os

import infra.basetest


class TestSyslogNg(infra.basetest.BRTest):
    config = infra.basetest.BASIC_TOOLCHAIN_CONFIG + \
        """
        LINGMO_PACKAGE_BUSYBOX_SHOW_OTHERS=y
        LINGMO_PACKAGE_SYSLOG_NG=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv5",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()

        cmd = "grep 'syslog-ng starting' /var/log/messages"
        self.assertRunOk(cmd)

        cmd = "logger my-message && "
        cmd += "sleep 1 && "
        cmd += "grep my-message /var/log/messages"
        self.assertRunOk(cmd)

        cmd = "syslog-ng-ctl reload && "
        cmd += "sleep 1"
        self.assertRunOk(cmd)

        cmd = "grep -i 'syslog-ng.*warning' /var/log/messages"
        _, exit_code = self.emulator.run(cmd)
        self.assertEqual(exit_code, 1)
