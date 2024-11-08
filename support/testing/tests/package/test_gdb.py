import os
import infra.basetest


class BaseGdb(infra.basetest.BRTest):
    def verify_host_gdb(self, prefix="arm-linux"):
        cmd = ["host/bin/%s-gdb" % prefix, "--version"]
        # We don't check the return value, as it automatically raises
        # an exception if the command returns with a non-zero value
        infra.run_cmd_on_host(self.builddir, cmd)

    def boot(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv5",
                           kernel="builtin",
                           options=["-initrd", img,
                                    "-net", "nic",
                                    "-net", "user"])
        self.emulator.login()

    def verify_gdbserver(self):
        cmd = "gdbserver --version"
        self.assertRunOk(cmd)

    def verify_gdb(self):
        cmd = "gdb --version"
        self.assertRunOk(cmd)


class TestGdbHostOnlyDefault(BaseGdb):
    config = \
        infra.basetest.MINIMAL_CONFIG + \
        """
        LINGMO_arm=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_PACKAGE_HOST_GDB=y
        """

    def test_run(self):
        self.verify_host_gdb()


class TestGdbHostOnlyAllFeatures(BaseGdb):
    config = \
        infra.basetest.MINIMAL_CONFIG + \
        """
        LINGMO_arm=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_PACKAGE_HOST_GDB=y
        LINGMO_PACKAGE_HOST_GDB_TUI=y
        LINGMO_PACKAGE_HOST_GDB_PYTHON3=y
        LINGMO_PACKAGE_HOST_GDB_SIM=y
        """

    def test_run(self):
        self.verify_host_gdb()


class TestGdbserverOnly(BaseGdb):
    config = \
        """
        LINGMO_arm=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_PACKAGE_GDB=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        self.boot()
        self.verify_gdbserver()


class TestGdbFullTarget(BaseGdb):
    config = \
        """
        LINGMO_arm=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_BOOTLIN=y
        LINGMO_PACKAGE_GDB=y
        LINGMO_PACKAGE_GDB_DEBUGGER=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def test_run(self):
        self.boot()
        self.verify_gdb()


class TestGdbArc(BaseGdb):
    config = \
        """
        LINGMO_arcle=y
        LINGMO_archs4x_rel31=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_PACKAGE_HOST_GDB=y
        LINGMO_PACKAGE_GDB=y
        LINGMO_PACKAGE_GDB_SERVER=y
        LINGMO_PACKAGE_GDB_DEBUGGER=y
        """

    def test_run(self):
        self.verify_host_gdb("arc-linux")
