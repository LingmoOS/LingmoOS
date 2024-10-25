from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3ServiceIdentity(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_SERVICE_IDENTITY=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """
    sample_scripts = ["tests/package/sample_python_service_identity.py"]
    timeout = 30
