from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Iptables(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_IPTABLES=y
        """
    sample_scripts = ["tests/package/sample_python_iptables.py"]
    timeout = 30
