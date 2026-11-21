find_package(GTest REQUIRED)
include(GoogleTest)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)

add_executable(files-tests files_test.cpp testing.h)
target_link_libraries(files-tests PRIVATE Lingmo::Files GTest::gtest_main)
gtest_discover_tests(files-tests DISCOVERY_TIMEOUT 30 PROPERTIES TIMEOUT 300)

add_executable(files-mimesniffer-tests mime_sniffer_test.cpp testing.h)
target_link_libraries(files-mimesniffer-tests PRIVATE Lingmo::Files GTest::gtest_main)
gtest_discover_tests(files-mimesniffer-tests)

add_executable(files-thumbnailers-tests thumbnailers_test.cpp testing.h)
target_link_libraries(files-thumbnailers-tests PRIVATE Lingmo::FilesThumbnails GTest::gtest_main)
gtest_discover_tests(files-thumbnailers-tests PROPERTIES TIMEOUT 120)

# UDisks2 is played on a private session bus.
add_executable(files-volumemanager-tests volume_manager_test.cpp testing.h)
pkg_check_modules(FilesTestSystemd REQUIRED IMPORTED_TARGET libsystemd)
target_link_libraries(files-volumemanager-tests PRIVATE Lingmo::Files GTest::gtest_main PkgConfig::FilesTestSystemd)
add_test(NAME files-volumemanager COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:files-volumemanager-tests>)
