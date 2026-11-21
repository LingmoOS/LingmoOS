# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(text-editor-core-tests
    test_main.cpp
    core_test.cpp
)
target_link_libraries(text-editor-core-tests PRIVATE editor-core GTest::gtest)
gtest_discover_tests(text-editor-core-tests)

add_executable(text-editor-ui-tests
    test_main.cpp
    view_test.cpp
    app_test.cpp
)
target_compile_definitions(text-editor-ui-tests PRIVATE TEXT_EDITOR_UI_TESTS)
target_link_libraries(text-editor-ui-tests PRIVATE editor-ui GTest::gtest)
gtest_discover_tests(text-editor-ui-tests)

add_executable(text-editor-capture capture.cpp)
target_link_libraries(text-editor-capture PRIVATE editor-ui)

# The editor on the live session with a scratch data folder: text-editor-live SCRATCH [FILE...].
add_executable(text-editor-live ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_compile_definitions(text-editor-live PRIVATE TEXT_EDITOR_LIVE_TEST)
target_link_libraries(text-editor-live PRIVATE editor-ui)
