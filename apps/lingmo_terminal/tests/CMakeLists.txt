find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(terminal-core-tests
    main.cpp
    emulation_test.cpp
    session_test.cpp
)
target_link_libraries(terminal-core-tests PRIVATE terminal-core GTest::gtest)
target_compile_definitions(terminal-core-tests PRIVATE
    KB_LAYOUTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../qmltermwidget/lib/kb-layouts"
)
gtest_discover_tests(terminal-core-tests)

# The windows and tabs on the headless platform.
add_executable(terminal-window-tests
    window_main.cpp
    window_test.cpp
    ../src/findbar.cpp
    ../src/processhelper.cpp
    ../src/settingsdialog.cpp
    ../src/terminalpage.cpp
    ../src/terminalsettings.cpp
    ../src/terminalwindow.cpp
    ../src/windows.cpp
)
target_include_directories(terminal-window-tests PRIVATE ../src)
target_link_libraries(terminal-window-tests PRIVATE terminal-view GTest::gtest)
target_compile_definitions(terminal-window-tests PRIVATE
    KB_LAYOUTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../qmltermwidget/lib/kb-layouts"
)
gtest_discover_tests(terminal-window-tests)

# Emulator comparison and throughput, against the earlier builds' tools.
add_executable(termdiff termdiff.cpp)
target_link_libraries(termdiff PRIVATE terminal-core)
add_executable(termbench termbench.cpp)
target_link_libraries(termbench PRIVATE terminal-core)
