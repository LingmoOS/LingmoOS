import os
import infra.basetest


class TestPolkitInfra(infra.basetest.BRTest):
    br2_external = [infra.filepath("tests/package/br2-external/polkit")]
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        LINGMO_PACKAGE_POLKIT=y
        LINGMO_PACKAGE_POLKIT_RULES_TEST=y
        """
    rule_paths = [
        "/etc/polkit-1/rules.d",
        "/usr/share/polkit-1/rules.d"
    ]

    def base_test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7", kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()


class TestPolkitSystemd(TestPolkitInfra):
    config = \
        """
        {}
        LINGMO_INIT_SYSTEMD=y
        LINGMO_PACKAGE_SYSTEMD_POLKIT=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """.format(TestPolkitInfra.config)

    def test_run(self):
        TestPolkitInfra.base_test_run(self)

        rule_file = "systemd-timesyncd-restart.rules"
        for rule_path in TestPolkitInfra.rule_paths:
            cmd = "su brtest -c '/bin/systemctl restart systemd-timesyncd.service'"
            _, exit_code = self.emulator.run(cmd, 10)
            self.assertNotEqual(exit_code, 0)

            cmd = "cp /root/{file} {path}".format(file=rule_file, path=rule_path)
            _, exit_code = self.emulator.run(cmd, 10)
            self.assertEqual(exit_code, 0)

            cmd = "su brtest -c '/bin/systemctl restart systemd-timesyncd.service'"
            _, exit_code = self.emulator.run(cmd, 10)
            self.assertEqual(exit_code, 0)

            cmd = "rm {path}/{file}".format(file=rule_file, path=rule_path)
            _, exit_code = self.emulator.run(cmd, 10)
            self.assertEqual(exit_code, 0)


class TestPolkitInitd(TestPolkitInfra):
    config = TestPolkitInfra.config

    def test_run(self):
        TestPolkitInfra.base_test_run(self)

        rule_file = "hello-polkit.rules"
        for rule_path in TestPolkitInfra.rule_paths:
            cmd = "su brtest -c 'pkexec hello-polkit'"
            output, exit_code = self.emulator.run(cmd, 10)
            self.assertEqual(exit_code, 127)
            self.assertEqual(output[0], "Error executing command as another user: Not authorized")

            cmd = "cp /root/{file} {path}/{file}".format(file=rule_file, path=rule_path)
            _, exit_code = self.emulator.run(cmd, 10)
            self.assertEqual(exit_code, 0)

            cmd = "su brtest -c 'pkexec hello-polkit'"
            output, exit_code = self.emulator.run(cmd, 10)
            self.assertEqual(exit_code, 0)
            self.assertEqual(output[0], "Hello polkit!")

            cmd = "rm {path}/{file}".format(file=rule_file, path=rule_path)
            _, exit_code = self.emulator.run(cmd, 10)
            self.assertEqual(exit_code, 0)
