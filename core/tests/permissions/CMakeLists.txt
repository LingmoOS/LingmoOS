cmake_minimum_required(VERSION 3.22)
project(permission_prompt_test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(LingmoSDK REQUIRED)
find_package(GTest REQUIRED)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)
set(PERMISSIONS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lingmo-services/permissions)
# The prompts' D-Bus service and windows, the windows headless.
add_executable(permission_prompt_test permission_prompt_test.cpp
    ${PERMISSIONS_DIR}/permissionmanager.cpp
    ${PERMISSIONS_DIR}/permissionprompt.cpp)
target_include_directories(permission_prompt_test PRIVATE ${PERMISSIONS_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(permission_prompt_test PRIVATE Lingmo::SDK GTest::gtest_main)
enable_testing()
add_test(NAME permission_prompt_test COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:permission_prompt_test>)
