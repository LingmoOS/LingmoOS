#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>
# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

import subprocess
import sys
import time
import unittest
from typing import Final

import gi
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from appium.webdriver.webdriver import ExtensionBase
from appium.webdriver.webelement import WebElement

gi.require_version('Gdk', '3.0')
from gi.repository import Gdk

WIDGET_ID: Final = "org.kde.plasma.volume"

class SetValueCommand(ExtensionBase):

    def method_name(self):
        return "set_value"

    def set_value(self, element: WebElement, value: str):
        """
        Set the value on this element in the application
        Args:
            value: The value to be set
        """
        data = {
            "id": element.id,
            "text": value,
        }
        return self.execute(data)["value"]

    def add_command(self):
        return "post", "/session/$sessionId/appium/element/$id/value"


class AppletTest(unittest.TestCase):
    """
    Tests for the widget
    """

    driver: webdriver.Remote | None = None
    pipewire: subprocess.Popen | None = None
    pulseaudio: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        """
        Initializes the webdriver
        """
        options = AppiumOptions()
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.nano {WIDGET_ID}")
        options.set_capability("environ", {
            "QT_FATAL_WARNINGS": "1",
            "QT_LOGGING_RULES": "org.kde.pulseaudio=false;org.kde.plasma.pulseaudio.warning=false;qt.accessibility.atspi.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false",
        })
        options.set_capability("timeouts", {'implicit': 30000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', extensions=[SetValueCommand], options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_volume_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        cls.driver.quit()
        if cls.pulseaudio is not None:
            cls.pulseaudio.terminate()
        if cls.pipewire is not None:
            cls.pipewire.terminate()

    def test_0_open(self) -> None:
        """
        Tests the widget can be opened
        """
        self.driver.find_element(AppiumBy.NAME, "No output or input devices found")

    def test_1_sink_1_list_device(self) -> None:
        """
        Add a null sink and adjust its volume
        """
        self.pipewire = subprocess.Popen(["pipewire"], stdout=sys.stderr, stderr=sys.stderr)
        time.sleep(1)
        self.pulseaudio = subprocess.Popen(["pipewire-pulse"], stdout=sys.stderr, stderr=sys.stderr)
        time.sleep(1)
        description: Final = "Virtual_Dummy_Output"
        subprocess.check_call(["pactl", "load-module", "module-null-sink", "sink_name=DummyOutput", f"sink_properties=node.nick={description}"])
        self.driver.find_element(AppiumBy.NAME, description)

        # Raise the maximum volume
        self.driver.find_element(AppiumBy.NAME, "Raise maximum volume").click()
        slider_element: WebElement = self.driver.find_element(AppiumBy.NAME, f"Adjust volume for {description}")
        self.driver.set_value(slider_element, str(int(0x10000 * 1.5)))  # PA_VOLUME_NORM * 1.5
        slider_element.click()
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "150%")))
        time.sleep(1)

        self.assertIn("150%", subprocess.check_output(["pactl", "list", "sinks"]).decode(encoding="utf-8"))


if __name__ == '__main__':
    unittest.main()
