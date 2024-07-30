from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Spake2(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_SPAKE2=y
        """
    sample_scripts = ["tests/package/sample_python_spake2.py"]
