from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Incremental(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_INCREMENTAL=y
        """
    sample_scripts = ["tests/package/sample_python_incremental.py"]
    timeout = 30
