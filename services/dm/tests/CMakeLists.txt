find_package(GTest REQUIRED)

add_executable(dm-test
    dm_test.cpp
    terminal_stub.cpp
    ../daemon/backgrounds.cpp
    ../daemon/config.cpp
    ../daemon/daemon.cpp
    ../daemon/greeterserver.cpp
    ../daemon/helperprocess.cpp
    ../daemon/sessions.cpp
    ../daemon/state.cpp
)
target_compile_definitions(dm-test PRIVATE
    DM_HELPER_PATH="/nonexistent/lingmo-dm-helper"
    DM_SESSION_WRAPPER="/nonexistent/lingmo-dm-session"
)
target_include_directories(dm-test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(dm-test PRIVATE lingmo-dm-common GTest::gtest)
add_test(NAME dm COMMAND dm-test)
