# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(system-monitor-tests
    main.cpp
    monitor_test.cpp
)
target_link_libraries(system-monitor-tests PRIVATE monitor-backend GTest::gtest)
# The application tests group every running copy of this binary.
gtest_discover_tests(system-monitor-tests PROPERTIES RESOURCE_LOCK system-monitor-tests)

add_executable(system-monitor-capture capture.cpp)
target_link_libraries(system-monitor-capture PRIVATE monitor-ui)

add_executable(system-monitor-ui-tests ui_test.cpp)
target_link_libraries(system-monitor-ui-tests PRIVATE monitor-ui GTest::gtest)
# The application tests group every running copy of this binary.
gtest_discover_tests(system-monitor-ui-tests PROPERTIES RESOURCE_LOCK system-monitor-ui-tests)
