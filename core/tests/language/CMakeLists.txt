cmake_minimum_required(VERSION 3.22)
project(language_service_test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(LingmoSDK REQUIRED)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
pkg_check_modules(Icu REQUIRED IMPORTED_TARGET icu-uc)
pkg_check_modules(Fontconfig REQUIRED IMPORTED_TARGET fontconfig)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(SERVICE_DIR ${CORE_DIR}/lingmo-services)
add_subdirectory(${CORE_DIR}/common ${CMAKE_CURRENT_BINARY_DIR}/common)
file(READ ${SERVICE_DIR}/appearance/fontconfig.conf FONTCONFIG_TEMPLATE)
configure_file(${SERVICE_DIR}/appearance/fontconfig_template.h.in ${CMAKE_CURRENT_BINARY_DIR}/fontconfig_template.h @ONLY)
add_executable(language_service_test language_service_test.cpp
    ${SERVICE_DIR}/locale/language.cpp
    ${SERVICE_DIR}/appearance/fontconfigwriter.cpp
    ${SERVICE_DIR}/notification.cpp)
target_include_directories(language_service_test PRIVATE ${SERVICE_DIR} ${CORE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(language_service_test PRIVATE lingmo-core-common Lingmo::SDKBase GTest::gtest_main
    PkgConfig::Systemd PkgConfig::Icu PkgConfig::Fontconfig)
enable_testing()
add_test(NAME language_service_test COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:language_service_test>)
