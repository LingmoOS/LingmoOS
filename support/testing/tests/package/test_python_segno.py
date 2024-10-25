from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Segno(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_SEGNO=y
        """
    sample_scripts = ["tests/package/sample_python_segno.py"]
