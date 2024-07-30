from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Libftdi1(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_LIBFTDI1=y
        LINGMO_PACKAGE_LIBFTDI1_PYTHON_BINDINGS=y
        """
    sample_scripts = ["tests/package/sample_libftdi1.py"]
    timeout = 40
