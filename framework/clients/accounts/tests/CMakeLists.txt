find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(accounts-client-tests accounts_test.cpp)
target_link_libraries(accounts-client-tests PRIVATE Lingmo::AccountsClient GTest::gtest_main)
# Each case owns org.freedesktop.Accounts on the shared session bus.
gtest_discover_tests(accounts-client-tests PROPERTIES RESOURCE_LOCK accounts-session-bus)
