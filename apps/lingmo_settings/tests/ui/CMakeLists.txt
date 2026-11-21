find_package(GTest REQUIRED)
include(GoogleTest)

# Each page adds its own <page>_test.cpp or <page>_<topic>_test.cpp, built
# with the page.
file(GLOB tests CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp)
set(SETTINGS_UI_TESTS)
foreach(test ${tests})
    get_filename_component(name ${test} NAME_WE)
    set(wanted FALSE)
    if(name STREQUAL "foundation_test")
        set(wanted TRUE)
    endif()
    foreach(page ${SETTINGS_BUILT_PAGES})
        if(name STREQUAL "${page}_test" OR name MATCHES "^${page}_.*_test$")
            set(wanted TRUE)
        endif()
    endforeach()
    if(wanted)
        list(APPEND SETTINGS_UI_TESTS ${test})
    endif()
endforeach()
add_executable(settings-ui-tests main.cpp ${SETTINGS_UI_TESTS})
target_compile_definitions(settings-ui-tests PRIVATE SETTINGS_SOURCE_DIR="${PROJECT_SOURCE_DIR}/src")
target_link_libraries(settings-ui-tests PRIVATE settings-ui GTest::gtest)
# Each test on a session bus of its own, where the pages' fakes own their
# services' names and the session's real services are out of reach.
if(CMAKE_VERSION VERSION_LESS 3.29)
    message(FATAL_ERROR "settings-ui-tests need CMake 3.29 for TEST_LAUNCHER")
endif()
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session REQUIRED)
set_target_properties(settings-ui-tests PROPERTIES TEST_LAUNCHER
    "${DBUS_RUN_SESSION_EXECUTABLE};--config-file=${CMAKE_CURRENT_SOURCE_DIR}/dbus-session.conf;--")
gtest_discover_tests(settings-ui-tests)

# Renders every page headless into PNGs, light and dark, at 100% and 150%.
add_executable(settings-capture capture.cpp)
target_compile_definitions(settings-capture PRIVATE SETTINGS_SOURCE_DIR="${PROJECT_SOURCE_DIR}/src")
target_link_libraries(settings-capture PRIVATE settings-ui)
