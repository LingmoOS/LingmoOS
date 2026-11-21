find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(timedate-client-tests time_date_test.cpp)
target_link_libraries(timedate-client-tests PRIVATE Lingmo::TimeDateClient GTest::gtest_main)
# Each case owns org.freedesktop.timedate1 on the shared session bus.
gtest_discover_tests(timedate-client-tests PROPERTIES RESOURCE_LOCK timedate-session-bus)
