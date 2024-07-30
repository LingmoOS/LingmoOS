from tests.init.base import InitSystemBase as InitSystemBase


class InitSystemOpenrcBase(InitSystemBase):
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_INIT_OPENRC=y
        LINGMO_TARGET_GENERIC_GETTY_PORT="ttyAMA0"
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def check_init(self):
        super(InitSystemOpenrcBase, self).check_init('/sbin/openrc-init')

        # Test all services are OK
        output, _ = self.emulator.run("rc-status -c")
        self.assertEqual(len(output), 0)


class TestInitSystemOpenrcRoFull(InitSystemOpenrcBase):
    config = InitSystemOpenrcBase.config + \
        """
        LINGMO_SYSTEM_DHCP="eth0"
        # LINGMO_TARGET_GENERIC_REMOUNT_ROOTFS_RW is not set
        LINGMO_TARGET_ROOTFS_SQUASHFS=y
        """

    def test_run(self):
        self.start_emulator("squashfs")
        self.check_init()


class TestInitSystemOpenrcRwFull(InitSystemOpenrcBase):
    config = InitSystemOpenrcBase.config + \
        """
        LINGMO_SYSTEM_DHCP="eth0"
        LINGMO_TARGET_ROOTFS_EXT2=y
        """

    def test_run(self):
        self.start_emulator("ext2")
        self.check_init()


class TestInitSystemOpenrcMergedUsrFull(InitSystemOpenrcBase):
    config = InitSystemOpenrcBase.config + \
        """
        LINGMO_ROOTFS_MERGED_USR=y
        LINGMO_SYSTEM_DHCP="eth0"
        LINGMO_TARGET_ROOTFS_EXT2=y
        """

    def test_run(self):
        self.start_emulator("ext2")
        self.check_init()
