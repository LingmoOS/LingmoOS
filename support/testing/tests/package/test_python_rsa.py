from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3RSA(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_RSA=y
        """
    sample_scripts = ["tests/package/sample_python_rsa.py"]
    timeout = 20
