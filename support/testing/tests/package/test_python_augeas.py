from tests.package.test_python import TestPythonPackageBase


class TestPythonAugeas(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_AUGEAS=y
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_AUGEAS=y
        """
    sample_scripts = ["tests/package/sample_python_augeas.py"]
    timeout = 60
