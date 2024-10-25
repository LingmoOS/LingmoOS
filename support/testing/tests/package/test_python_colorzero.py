from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Colorzero(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_COLORZERO=y
        """
    sample_scripts = ["tests/package/sample_python_colorzero.py"]
    timeout = 30
