#!/usr/bin/env python3

# Execute tests with the following command:
# selenium-webdriver-at-spi-run appiumtests/about_this_system.py

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
# SPDX-FileCopyrightText: 2023 Alexander Wilms <f.alexander.wilms@gmail.com>

import logging
import sys
import unittest

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy


class AboutThisSystemTests(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        options = AppiumOptions()
        # The app capability may be a command line or a desktop file id.
        options.set_capability("app", "org.kde.kinfocenter.desktop")
        options.set_capability("timeouts", {"implicit": 2 * 10 * 60 * 1000})
        # Boilerplate, always the same
        self.driver = webdriver.Remote(
            command_executor="http://127.0.0.1:4723", options=options
        )
        logging.getLogger().setLevel(logging.INFO)

    @classmethod
    def tearDownClass(self):
        # Make sure to terminate the driver again, lest it dangles.
        self.driver.quit()

    def setUp(self):
        pass

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(
                f"failed_test_shot_kinfocenter_kcms_#{self.id()}.png"
            )

    def test_About_this_System(self):
        self.driver.find_element(by=AppiumBy.NAME, value="About this System").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Copy Details").click()
        text = self.driver.get_clipboard_text()
        logging.info("▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼")
        logging.info("test_About_this_System():")
        logging.info(text)
        logging.info("▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲")
        self.driver.get_screenshot_as_file(
            f"appium_artifact_screenshot_{sys._getframe().f_code.co_name}.png"
        )
        self.assertIn(
            "Graphics Platform: Wayland",
            text,
            "'Graphics Platform: Wayland' not found in 'Abou this System' KCM",
        )

    def test_Devices_CPU(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Devices").click()
        self.driver.find_element(by=AppiumBy.NAME, value="CPU").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Copy to Clipboard").click()
        clipboard_contents = self.driver.get_clipboard_text()
        logging.info("▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼")
        logging.info("test_Devices_CPU():")
        logging.info(clipboard_contents)
        logging.info("▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲")
        self.driver.get_screenshot_as_file(
            f"appium_artifact_screenshot_{sys._getframe().f_code.co_name}.png"
        )
        self.assertIn("64-bit", clipboard_contents, "'64-bit' not found in CPU KCM")

    def test_Graphics_Wayland(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Graphics").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Wayland").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Copy to Clipboard").click()
        clipboard_contents = self.driver.get_clipboard_text()
        logging.info("▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼")
        logging.info("test_Graphics_Wayland():")
        logging.info(clipboard_contents)
        logging.info("▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲")
        self.driver.get_screenshot_as_file(
            f"appium_artifact_screenshot_{sys._getframe().f_code.co_name}.png"
        )
        self.assertIn(
            "interface: 'wl_compositor'",
            clipboard_contents,
            "'wl_compositor' not found in Wayland KCM",
        )


if __name__ == "__main__":
    unittest.main()
