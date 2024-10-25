from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Ubjson(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_UBJSON=y
        """
    sample_scripts = ["tests/package/sample_python_ubjson_enc.py",
                      "tests/package/sample_python_ubjson_dec.py"]
