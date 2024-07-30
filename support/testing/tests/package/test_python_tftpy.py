from tests.package.test_python import TestPythonPackageBase


class TestPythonTftpy(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_TFTPY=y
        """
    sample_scripts = ["tests/package/sample_python_tftpy.py"]
