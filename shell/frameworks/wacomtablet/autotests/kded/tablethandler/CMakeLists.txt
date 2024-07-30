add_executable(Test.KDED.TabletHandler testtablethandler.cpp ../kdedtestutils.cpp ../tabletbackendmock.cpp)
add_test(NAME Test.KDED.TabletHandler COMMAND Test.KDED.TabletHandler)
ecm_mark_as_test(Test.KDED.TabletHandler)
target_link_libraries(Test.KDED.TabletHandler ${WACOM_KDED_TEST_LIBS})

configure_file(testtablethandler.configrc ${CMAKE_CURRENT_BINARY_DIR}/testtablethandler.configrc COPYONLY)
configure_file(testtablethandler.profilesrc ${CMAKE_CURRENT_BINARY_DIR}/testtablethandler.profilesrc COPYONLY)
