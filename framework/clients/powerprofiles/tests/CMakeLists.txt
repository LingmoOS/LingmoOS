find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(powerprofiles-client-tests power_profiles_test.cpp)
target_link_libraries(powerprofiles-client-tests PRIVATE Lingmo::PowerProfilesClient GTest::gtest_main)
# Each case owns org.freedesktop.UPower.PowerProfiles on the shared session bus.
gtest_discover_tests(powerprofiles-client-tests PROPERTIES RESOURCE_LOCK powerprofiles-session-bus)
