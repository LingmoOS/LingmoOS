import os
from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Gobject(TestPythonPackageBase):
    __test__ = True
    config = \
        """
        LINGMO_arm=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_cortex_a9=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_PACKAGE_GOBJECT_INTROSPECTION=y
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_GOBJECT=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def login(self):
        img = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", img])
        self.emulator.login()

    sample_scripts = ["tests/package/sample_python_gobject.py"]
