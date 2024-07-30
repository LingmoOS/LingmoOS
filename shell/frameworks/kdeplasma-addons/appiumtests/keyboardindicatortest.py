#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import subprocess
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy

WIDGET_ID: Final = "org.kde.plasma.keyboardindicator"


class KeyboardIndicatorTest(unittest.TestCase):

    driver: webdriver.Remote

    @classmethod
    def setUpClass(cls) -> None:
        options = AppiumOptions()
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.nano {WIDGET_ID}")
        options.set_capability("environ", {
            "QT_FATAL_WARNINGS": "1",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;qt.dbus.integration.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false",
        })
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    @classmethod
    def tearDownClass(cls) -> None:
        # Make sure to terminate the driver again, lest it dangles.
        cls.driver.quit()

    def tearDown(self) -> None:
        pass

    def test_1_caps_lock(self) -> None:
        self.driver.find_element(AppiumBy.NAME, "No lock keys activated")
        subprocess.check_call(["xdotool", "key", "Caps_Lock"])
        self.driver.find_element(AppiumBy.NAME, "Caps Lock activated")
        subprocess.check_call(["xdotool", "key", "Caps_Lock"])
        self.driver.find_element(AppiumBy.NAME, "No lock keys activated")

    def test_2_num_lock(self) -> None:
        """
        Num lock indicator is disabled by default
        """
        self.driver.find_element(AppiumBy.NAME, "No lock keys activated")
        subprocess.check_call(["xdotool", "key", "Num_Lock"])
        self.driver.find_element(AppiumBy.NAME, "No lock keys activated")
        subprocess.check_call(["xdotool", "key", "Num_Lock"])
        self.driver.find_element(AppiumBy.NAME, "No lock keys activated")


if __name__ == '__main__':
    unittest.main()
