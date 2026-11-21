find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(upower-client-tests upower_test.cpp)
target_link_libraries(upower-client-tests PRIVATE Lingmo::UPowerClient GTest::gtest_main)
# Each case owns org.freedesktop.UPower on the shared session bus.
gtest_discover_tests(upower-client-tests PROPERTIES RESOURCE_LOCK upower-session-bus)
