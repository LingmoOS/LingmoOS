# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

if(NOT Qt${QT_MAJOR_VERSION}_VERSION VERSION_GREATER_EQUAL "6.5.0")
    # Before Qt 6.5 the AbstractButtons didn't trigger properly at all.
    return()
endif()

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
if(NOT SeleniumWebDriverATSPI_FOUND)
    return()
endif()

add_subdirectory(calendarplugins)

add_test(
    NAME calculatortest
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/calculatortest.py --failfast
)
set_tests_properties(calculatortest PROPERTIES TIMEOUT 300 ENVIRONMENT "TEST_WITH_KWIN_WAYLAND=1")

add_test(
    NAME keyboardindicatortest
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/keyboardindicatortest.py --failfast
)
set_tests_properties(keyboardindicatortest PROPERTIES TIMEOUT 300 ENVIRONMENT "TEST_WITH_KWIN_WAYLAND=0")
