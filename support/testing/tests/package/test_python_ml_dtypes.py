from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3MlDtypes(TestPythonPackageBase):
    __test__ = True

    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_ML_DTYPES=y
        """
    sample_scripts = ["tests/package/sample_python_ml_dtypes.py"]
    timeout = 20
