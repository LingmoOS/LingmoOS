cmake_minimum_required(VERSION 3.22)
project(night_light_test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(LingmoSDK REQUIRED)
find_package(LingmoClients REQUIRED COMPONENTS display)
find_package(GTest REQUIRED)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)
set(SERVICE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lingmo-services)
add_executable(night_light_test night_light_test.cpp
    ${SERVICE_DIR}/nightlight/nightlight.cpp
    ${SERVICE_DIR}/schedule/scheduledfeature.cpp
    ${SERVICE_DIR}/schedule/scheduleutils.cpp)
target_include_directories(night_light_test PRIVATE ${SERVICE_DIR} ${SERVICE_DIR}/nightlight
    ${SERVICE_DIR}/schedule ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(night_light_test PRIVATE Lingmo::DisplayClient Lingmo::SDKBase GTest::gtest_main)
enable_testing()
add_test(NAME night_light_test COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:night_light_test>)
