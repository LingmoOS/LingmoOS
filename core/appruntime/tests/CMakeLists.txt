cmake_minimum_required(VERSION 3.22)
project(lingmo-appruntime-tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(LingmoSDK REQUIRED)
find_package(GTest REQUIRED)
include(GoogleTest)
find_package(PkgConfig REQUIRED)
enable_testing()

add_subdirectory(../../common ${CMAKE_CURRENT_BINARY_DIR}/common)

add_executable(application-scope-test application_scope_test.cpp ../applicationscope.cpp)
target_include_directories(application-scope-test PRIVATE ..)
target_link_libraries(application-scope-test PRIVATE lingmo-core-common Lingmo::SDKBase GTest::gtest_main)
add_test(NAME application-scope COMMAND application-scope-test)

add_executable(chromium-switches-test chromium_switches_test.cpp ../chromiumswitches.cpp)
target_include_directories(chromium-switches-test PRIVATE ..)
target_link_libraries(chromium-switches-test PRIVATE Lingmo::SDKBase GTest::gtest_main)
add_test(NAME chromium-switches COMMAND chromium-switches-test)

# The runtime itself, on a private bus.
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
add_executable(lingmo-appruntime ../main.cpp ../appruntime.cpp ../applicationscope.cpp ../chromiumswitches.cpp)
target_include_directories(lingmo-appruntime PRIVATE ..)
target_link_libraries(lingmo-appruntime PRIVATE lingmo-core-common Lingmo::SDKBase PkgConfig::Systemd)

add_executable(runtime-dbus-test runtime_dbus_test.cpp)
target_link_libraries(runtime-dbus-test PRIVATE Lingmo::SDKBase GTest::gtest_main)
target_compile_definitions(runtime-dbus-test PRIVATE APPRUNTIME_BINARY="$<TARGET_FILE:lingmo-appruntime>")
add_dependencies(runtime-dbus-test lingmo-appruntime)
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session REQUIRED)
add_test(NAME runtime-dbus COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} -- $<TARGET_FILE:runtime-dbus-test>)
