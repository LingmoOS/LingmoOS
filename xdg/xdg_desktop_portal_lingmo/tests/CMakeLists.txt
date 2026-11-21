find_package(GTest REQUIRED)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)

add_executable(portal-test portal_test.cpp)
target_link_libraries(portal-test PRIVATE portal-core GTest::gtest)
target_compile_definitions(portal-test PRIVATE
    PORTAL_EXECUTABLE="$<TARGET_FILE:xdg-desktop-portal-lingmo>"
    PORTAL_DATA_DIR="${PROJECT_SOURCE_DIR}/data"
)
add_dependencies(portal-test xdg-desktop-portal-lingmo)

add_test(NAME portal
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_session.sh ${DBUS_RUN_SESSION} $<TARGET_FILE:portal-test>)
set_tests_properties(portal PROPERTIES TIMEOUT 120)
