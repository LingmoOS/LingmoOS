find_package(GTest REQUIRED)

add_executable(display-client-tests
    display_client_test.cpp
    output_model_test.cpp
)
set_target_properties(display-client-tests PROPERTIES AUTOMOC OFF AUTORCC OFF)
target_link_libraries(display-client-tests PRIVATE Lingmo::DisplayClient GTest::gtest_main)
# The client and its fake service talk over a private session bus.
add_test(NAME display-client-tests COMMAND dbus-run-session -- $<TARGET_FILE:display-client-tests>)
