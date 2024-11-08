import os

from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3SciPy(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_SCIPY=y
        LINGMO_TARGET_ROOTFS_EXT2=y
        LINGMO_TARGET_ROOTFS_EXT2_SIZE="250M"
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """
    sample_scripts = ["tests/package/sample_python_scipy.py"]
    timeout = 30

    def login(self):
        ext2_file = os.path.join(self.builddir, "images", "rootfs.ext2")
        self.emulator.boot(arch="armv5",
                           kernel="builtin",
                           options=["-drive", "file=%s,if=scsi,format=raw" % ext2_file],
                           kernel_cmdline=["rootwait", "root=/dev/sda"])
        self.emulator.login()
