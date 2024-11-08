import os
import time

from tests.package.test_python import TestPythonPackageBase


class TestPythonPy3Fastapi(TestPythonPackageBase):
    """Test fastapi, uvicorn and pydantic2.

    fastapi needs an asgi server to run. Since we select uvicorn as
    asgi server here, uvicorn is tested as well.

    pydantic is an major dependency of fastapi so it is implicitly
    tested here as well.
    """
    __test__ = True
    config = \
        """
        LINGMO_arm=y
        LINGMO_cortex_a9=y
        LINGMO_ARM_ENABLE_NEON=y
        LINGMO_ARM_ENABLE_VFP=y
        LINGMO_TOOLCHAIN_EXTERNAL=y
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_FASTAPI=y
        LINGMO_PACKAGE_PYTHON_UVICORN=y
        LINGMO_TARGET_ROOTFS_CPIO=y
        # LINGMO_TARGET_ROOTFS_TAR is not set
        """
    sample_scripts = ["tests/package/sample_python_fastapi.py"]
    timeout = 60

    def test_run(self):
        self.login()
        self.check_sample_scripts_exist()
        cmd = "uvicorn sample_python_fastapi:app > /dev/null 2>&1 &"

        _, exit_code = self.emulator.run(cmd, timeout=self.timeout)

        # Give enough time for the uvicorn server to start up
        for attempt in range(30):
            time.sleep(1)

            cmd = "wget -q -O - http://127.0.0.1:8000/"
            output, exit_code = self.emulator.run(cmd, timeout=self.timeout)
            if exit_code == 0:
                self.assertEqual(output[0], '{"message":"Hello World"}')
                break
        else:
            self.assertTrue(False, "Timeout while waiting for fastapi server")

    def login(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()
