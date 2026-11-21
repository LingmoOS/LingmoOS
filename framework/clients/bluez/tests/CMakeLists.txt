find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

add_executable(bluez-client-tests manager_test.cpp)
set_target_properties(bluez-client-tests PROPERTIES AUTOMOC OFF AUTORCC OFF)
target_link_libraries(bluez-client-tests PRIVATE Lingmo::BluezClient PkgConfig::Systemd GTest::gtest_main)
# The manager and the fake BlueZ talk over a private session bus.
add_test(NAME bluez-client-tests COMMAND dbus-run-session -- $<TARGET_FILE:bluez-client-tests>)
