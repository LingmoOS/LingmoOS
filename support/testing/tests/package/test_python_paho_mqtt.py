from tests.package.test_python import TestPythonPackageBase
import os


class TestPythonPahoMQTT(TestPythonPackageBase):
    __test__ = True
    config = TestPythonPackageBase.config + \
        """
        LINGMO_PACKAGE_MOSQUITTO=y
        LINGMO_PACKAGE_MOSQUITTO_BROKER=y
        LINGMO_PACKAGE_PYTHON3=y
        LINGMO_PACKAGE_PYTHON_PAHO_MQTT=y
        """
    sample_scripts = ["tests/package/sample_python_paho_mqtt.py"]

    def test_run(self):
        self.login()
        self.check_sample_scripts_exist()

        cmd = "%s %s" % (self.interpreter, os.path.basename(self.sample_scripts[0]))
        output, exit_code = self.emulator.run(cmd)
        self.assertEqual(exit_code, 0)
        self.assertEqual(output[0], "Hello, World!")
