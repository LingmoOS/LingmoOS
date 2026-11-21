# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.22)
project(session_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(LingmoSDK REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
include(GoogleTest)
enable_testing()

set(SESSION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
add_subdirectory(${CORE_DIR}/common ${CMAKE_CURRENT_BINARY_DIR}/common)

function(session_test name)
    cmake_parse_arguments(arg "" "" "SOURCES" ${ARGN})
    add_executable(${name}-test ${arg_SOURCES})
    target_include_directories(${name}-test PRIVATE ${SESSION_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${name}-test PRIVATE lingmo-core-common Lingmo::SDKBase Qt6::Core GTest::gtest)
    add_test(NAME ${name} COMMAND ${name}-test)
endfunction()

pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

session_test(session-lifecycle SOURCES lifecycle_test.cpp ${SESSION_DIR}/compositorprocess.cpp
             ${SESSION_DIR}/journalstream.cpp)
target_link_libraries(session-lifecycle-test PRIVATE PkgConfig::Systemd)
session_test(shell-lifecycle SOURCES shell_lifecycle_test.cpp ${SESSION_DIR}/shellprocess.cpp
             ${SESSION_DIR}/shellconnection.cpp ${SESSION_DIR}/journalstream.cpp)
target_link_libraries(shell-lifecycle-test PRIVATE PkgConfig::Systemd)
session_test(journal-stream SOURCES journal_stream_test.cpp ${SESSION_DIR}/journalstream.cpp)
target_link_libraries(journal-stream-test PRIVATE PkgConfig::Systemd GTest::gtest_main)
session_test(xwayland-environment SOURCES xwayland_environment_test.cpp ${SESSION_DIR}/xwaylandenvironment.cpp)
target_link_libraries(xwayland-environment-test PRIVATE GTest::gtest_main)
session_test(autostart SOURCES autostart_test.cpp ${SESSION_DIR}/autostartentries.cpp)
target_link_libraries(autostart-test PRIVATE Qt6::Core GTest::gtest_main)
session_test(user-directories SOURCES user_directories_test.cpp ${SESSION_DIR}/userdirectories.cpp)
target_link_libraries(user-directories-test PRIVATE Qt6::Core GTest::gtest_main)
session_test(restart-policy SOURCES restart_policy_test.cpp ${SESSION_DIR}/restartpolicy.cpp)
target_link_libraries(restart-policy-test PRIVATE GTest::gtest_main)
