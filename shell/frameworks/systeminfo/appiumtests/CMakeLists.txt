# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

if(NOT BUILD_TESTING OR NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
    return()
endif()

find_package(SeleniumWebDriverATSPI)
set_package_properties(SeleniumWebDriverATSPI PROPERTIES
    DESCRIPTION "Server component for selenium tests using Linux accessibility infrastructure"
    PURPOSE "Needed for GUI tests"
    URL "https://invent.kde.org/sdk/selenium-webdriver-at-spi"
    TYPE OPTIONAL
)

if(NOT SeleniumWebDriverATSPI_FOUND AND NOT "$ENV{KDECI_BUILD}" STREQUAL "TRUE") # always run on CI
    return()
endif()

add_test(
    NAME kinfocenter_appiumtests
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/kinfocenter_appiumtests.py
)
set_tests_properties(kinfocenter_appiumtests PROPERTIES TIMEOUT 1200)

