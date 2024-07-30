#!/usr/bin/env python3

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2016 Microsoft Corporation. All rights reserved.
# SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from selenium.webdriver.support.ui import WebDriverWait

WIDGET_ID: Final = "org.kde.plasma.calculator"


class SimpleCalculatorTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        options = AppiumOptions()
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.nano {WIDGET_ID}")
        options.set_capability("environ", {
            "QT_FATAL_WARNINGS": "1",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false;kf.coreaddons.warning=false",
        })
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def setUp(self):
        self.driver.find_element(by=AppiumBy.NAME, value="AC").click()
        wait = WebDriverWait(self.driver, 5)
        wait.until(lambda x: self.getresults() == '0')

    def tearDown(self):
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file("failed_test_shot_{}.png".format(self.id()))

    @classmethod
    def tearDownClass(self):
        self.driver.quit()

    def getresults(self):
        displaytext = self.driver.find_element(by='description', value="Result").text
        return displaytext

    def test_initialize(self):
        self.driver.find_element(by=AppiumBy.NAME, value="AC").click()
        self.driver.find_element(by=AppiumBy.NAME, value="7").click()
        self.assertEqual(self.getresults(), "7")

    def test_addition(self):
        self.driver.find_element(by=AppiumBy.NAME, value="1").click()
        self.driver.find_element(by=AppiumBy.NAME, value="+").click()
        self.driver.find_element(by=AppiumBy.NAME, value="7").click()
        self.driver.find_element(by=AppiumBy.NAME, value="=").click()
        self.assertEqual(self.getresults(), "8")

    def test_combination(self):
        self.driver.find_element(by=AppiumBy.NAME, value="7").click()
        self.driver.find_element(by=AppiumBy.NAME, value="×").click()
        self.driver.find_element(by=AppiumBy.NAME, value="9").click()
        self.driver.find_element(by=AppiumBy.NAME, value="+").click()
        self.driver.find_element(by=AppiumBy.NAME, value="1").click()
        self.driver.find_element(by=AppiumBy.NAME, value="=").click()
        self.driver.find_element(by=AppiumBy.NAME, value="÷").click()
        self.driver.find_element(by=AppiumBy.NAME, value="8").click()
        self.driver.find_element(by=AppiumBy.NAME, value="=").click()
        self.assertEqual(self.getresults(), "8")

    def test_division(self):
        self.driver.find_element(by=AppiumBy.NAME, value="8").click()
        self.driver.find_element(by=AppiumBy.NAME, value="8").click()
        self.driver.find_element(by=AppiumBy.NAME, value="÷").click()
        self.driver.find_element(by=AppiumBy.NAME, value="1").click()
        self.driver.find_element(by=AppiumBy.NAME, value="1").click()
        self.driver.find_element(by=AppiumBy.NAME, value="=").click()
        self.assertEqual(self.getresults(), "8")

    def test_multiplication(self):
        self.driver.find_element(by=AppiumBy.NAME, value="9").click()
        self.driver.find_element(by=AppiumBy.NAME, value="×").click()
        self.driver.find_element(by=AppiumBy.NAME, value="8").click()
        self.driver.find_element(by=AppiumBy.NAME, value="=").click()
        self.assertEqual(self.getresults(), "72")

    def test_subtraction(self):
        self.driver.find_element(by=AppiumBy.NAME, value="9").click()
        self.driver.find_element(by=AppiumBy.NAME, value="-").click()
        self.driver.find_element(by=AppiumBy.NAME, value="1").click()
        self.driver.find_element(by=AppiumBy.NAME, value="=").click()
        self.assertEqual(self.getresults(), "8")


if __name__ == '__main__':
    unittest.main()
