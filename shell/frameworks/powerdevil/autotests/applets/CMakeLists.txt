# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

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
if(NOT SeleniumWebDriverATSPI_FOUND AND NOT DEFINED ENV{KDECI_BUILD})
    return()
endif()

add_test(
    NAME batterymonitortest_withdisplaydevice
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/batterymonitortest.py --failfast
)
set_tests_properties(batterymonitortest_withdisplaydevice PROPERTIES TIMEOUT 120 ENVIRONMENT "ENABLE_DISPLAY_DEVICE=0;POWERDEVIL_PATH=${KDE_INSTALL_FULL_LIBEXECDIR}/org_kde_powerdevil")

add_test(
    NAME batterymonitortest_withoutdisplaydevice
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/batterymonitortest.py --failfast
)
set_tests_properties(batterymonitortest_withoutdisplaydevice PROPERTIES TIMEOUT 120 ENVIRONMENT "ENABLE_DISPLAY_DEVICE=0;POWERDEVIL_PATH=${KDE_INSTALL_FULL_LIBEXECDIR}/org_kde_powerdevil")

execute_process(COMMAND gcc -print-file-name=libasan.so OUTPUT_VARIABLE LIBASAN_PATH ECHO_OUTPUT_VARIABLE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND gcc -print-file-name=libumockdev-preload.so.0 OUTPUT_VARIABLE LIBUMOCKDEV_PATH ECHO_OUTPUT_VARIABLE OUTPUT_STRIP_TRAILING_WHITESPACE)
add_test(
    NAME brightnesstest
    COMMAND selenium-webdriver-at-spi-run env LD_PRELOAD=${LIBASAN_PATH}:${LIBUMOCKDEV_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/brightnesstest.py --failfast
)
set_tests_properties(brightnesstest PROPERTIES TIMEOUT 120 ENVIRONMENT "TEST_WITH_KWIN_WAYLAND=0;POWERDEVIL_PATH=${KDE_INSTALL_FULL_LIBEXECDIR}/org_kde_powerdevil;BACKLIGHTHELPER_PATH=${CMAKE_INSTALL_PREFIX}/${KAUTH_HELPER_INSTALL_DIR}/backlighthelper;KDE_INSTALL_DBUSINTERFACEDIR=${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_DBUSINTERFACEDIR}")
