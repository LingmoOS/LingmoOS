# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(mountd-tests diskimage_test.cpp mtpmonitor_test.cpp remoteurl_test.cpp)
target_link_libraries(mountd-tests PRIVATE mountd-core GTest::gtest_main)
target_compile_definitions(mountd-tests PRIVATE FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
gtest_discover_tests(mountd-tests)

add_executable(fileoperationsdaemon-tests fileoperationsdaemon_test.cpp)
target_link_libraries(fileoperationsdaemon-tests PRIVATE Lingmo::SDKBase PkgConfig::LibArchive GTest::gtest_main)
target_compile_definitions(fileoperationsdaemon-tests PRIVATE
    FILEMANAGER_DAEMON="$<TARGET_FILE:lingmo-filemanager-daemon>")
add_dependencies(fileoperationsdaemon-tests lingmo-filemanager-daemon)
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session REQUIRED)
# A bus without service directories, so the installed daemon cannot be
# activated before the build tree's owns the name.
add_test(NAME fileoperationsdaemon
         COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} --config-file=${CMAKE_CURRENT_SOURCE_DIR}/dbus-session.conf
                 -- $<TARGET_FILE:fileoperationsdaemon-tests>)

add_executable(filemanager-capture capture.cpp)
target_link_libraries(filemanager-capture PRIVATE fm-ui)

# The models and the UI on a headless application, on a bus of their own.
add_executable(filemanager-tests testmain.cpp chooser_test.cpp foldermodel_test.cpp places_test.cpp preview_test.cpp
    desktopmenu_test.cpp desktopwidgets_test.cpp launcher_test.cpp positioner_test.cpp shortcut_test.cpp header_test.cpp)
pkg_check_modules(FilemanagerTestSystemd REQUIRED IMPORTED_TARGET libsystemd)
target_link_libraries(filemanager-tests PRIVATE fm-ui GTest::gtest PkgConfig::FilemanagerTestSystemd PkgConfig::LibArchive)
target_compile_definitions(filemanager-tests PRIVATE
    FILEMANAGER_DAEMON="$<TARGET_FILE:lingmo-filemanager-daemon>")
add_dependencies(filemanager-tests lingmo-filemanager-daemon)
add_test(NAME filemanager
         COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} --config-file=${CMAKE_CURRENT_SOURCE_DIR}/dbus-session.conf
                 -- $<TARGET_FILE:filemanager-tests>)

# The listing benchmark: a folder listed and sorted, timed.
add_executable(fmbench fmbench.cpp)
target_link_libraries(fmbench PRIVATE fm-core)
