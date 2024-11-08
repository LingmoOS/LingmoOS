from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Cbor(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_CBOR=y
        """
    sample_scripts = ["tests/package/sample_python_cbor_enc.py",
                      "tests/package/sample_python_cbor_dec.py"]
