include_directories (${CMAKE_BINARY_DIR}/src/common)

add_executable(Test.KDED.DBusTabletService testdbustabletservice.cpp ../tablethandlermock.cpp)
add_test(NAME Test.KDED.DBusTabletService COMMAND Test.KDED.DBusTabletService)
ecm_mark_as_test(Test.KDED.DBusTabletService)
target_link_libraries(Test.KDED.DBusTabletService ${WACOM_KDED_TEST_LIBS})
