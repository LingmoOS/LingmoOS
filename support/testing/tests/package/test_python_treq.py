from tests.package.test_python import TestPythonPackageBase


class TestPythonTreq(TestPythonPackageBase):
    sample_scripts = ["tests/package/sample_python_treq.py"]

    def run_sample_scripts(self):
        cmd = self.interpreter + " sample_python_treq.py"
        output, exit_code = self.emulator.run(cmd, timeout=20)
        refuse_msgs = [1 for line in output if "Connection refused" in line]
        self.assertGreater(sum(refuse_msgs), 0)
        self.assertEqual(exit_code, 0)


class TestPythonPy3Treq(TestPythonTreq):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_TREQ=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """
