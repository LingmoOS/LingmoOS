import os
from tests.package.test_python import TestPythonPackageBase


class TestGst1Python(TestPythonPackageBase):
    __test__ = True
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_TOOLCHAIN_EXTERNAL_LINARO_ARM=y
        LINGMO_PACKAGE_GSTREAMER1=y
        LINGMO_PACKAGE_GST1_PLUGINS_BASE_PLUGIN_VIDEOTESTSRC=y
        LINGMO_PACKAGE_GST1_PLUGINS_BAD=y
        LINGMO_PACKAGE_GST1_PLUGINS_BAD_PLUGIN_DEBUGUTILS=y
        LINGMO_PACKAGE_GST1_PYTHON=y
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """

    def login(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()
    sample_scripts = ["tests/package/sample_gst1_python.py"]
    timeout = 200
