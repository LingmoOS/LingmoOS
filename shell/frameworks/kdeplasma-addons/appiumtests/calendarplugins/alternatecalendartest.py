#!/usr/bin/env python3

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>

import subprocess
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.remote.webelement import WebElement
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

WIDGET_ID: Final = "org.kde.plasma.digitalclock"


class AlternateCalendarTests(unittest.TestCase):

    driver: webdriver.Remote

    @classmethod
    def setUpClass(cls) -> None:
        options = AppiumOptions()
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.desktop {WIDGET_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
        })
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def setUp(self) -> None:
        pass

    def tearDown(self) -> None:
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_alternatecalendartest_{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        cls.driver.quit()

    def test_1_config_dialog_1_enable_plugin(self) -> None:
        """
        Opens the config dialog and enables the plugin
        """
        subprocess.check_call(["plasmawindowed", "--config"])
        wait = WebDriverWait(self.driver, 10)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Date format:")))

        self.driver.find_element(AppiumBy.NAME, "Calendar").click()
        wait = WebDriverWait(self.driver, 10)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Available Plugins:")))
        # Enable the plugin
        plugin_checkbox: WebElement = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Alternate Calendar")))
        plugin_checkbox.click()
        self.driver.find_element(AppiumBy.NAME, "Apply").click()
        # Switch back to Appearance so there will only be one "Alternate Calendar" match
        self.driver.find_element(AppiumBy.NAME, "Appearance").click()
        wait.until_not(lambda _: plugin_checkbox.is_displayed())
        # Switch to the calendar plugin
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Alternate Calendar"))).click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Calendar system:")))
        combobox_element = self.driver.find_element(AppiumBy.NAME, "Julian")
        combobox_element.click()  # index + 1
        combobox_element.click()  # index + 1
        self.driver.find_element(AppiumBy.NAME, "Chinese Lunar Calendar")

        # Close the config dialog
        self.driver.find_element(AppiumBy.NAME, "OK").click()
        wait.until_not(lambda _: combobox_element.is_displayed())

        # Open the widget and check if the sublabel is sucessfully loaded
        self.driver.find_element(AppiumBy.ACCESSIBILITY_ID, "digital-clock-compactrepresentation").click()
        try:
            self.driver.find_element(AppiumBy.NAME, "廿一")
        except NoSuchElementException:
            self.driver.find_element(AppiumBy.NAME, "廿二")


if __name__ == '__main__':
    unittest.main()
