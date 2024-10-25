import os

from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3PytestAsyncio(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_PYTEST=y
        LINGMO_PACKAGE_PYTHON_PYTEST_ASYNCIO=y
        """
    sample_scripts = ["tests/package/sample_python_pytest_asyncio.py"]

    def run_sample_scripts(self):
        for script in self.sample_scripts:
            cmd = self.interpreter + " -m pytest " + os.path.basename(script)
            self.assertRunOk(cmd, timeout=self.timeout)
