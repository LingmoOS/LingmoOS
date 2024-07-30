from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3RpiGpio(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_RPI_GPIO=y
        """
    sample_scripts = ["tests/package/sample_python_rpi_gpio.py"]
