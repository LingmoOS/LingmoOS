from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Mako(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_MAKO=y
        """
    sample_scripts = ["tests/package/sample_python_mako.py"]
    timeout = 30


class TestPythonPy3MakoExt(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_MAKO=y
        LINGMO_PACKAGE_PYTHON_MAKO_EXT_PYGMENTPLUGIN=y
        LINGMO_PACKAGE_PYTHON_MAKO_EXT_BABELPLUGIN=y
        """
    sample_scripts = ["tests/package/sample_python_mako_ext.py"]
    timeout = 30
