from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Qrcode(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_QRCODE=y
        LINGMO_PACKAGE_PYTHON_QRCODE_SVG=y
        """
    sample_scripts = ["tests/package/sample_python_qrcode.py"]
