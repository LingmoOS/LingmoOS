add_executable(Test.KDED.TabletBackend testtabletbackend.cpp ../kdedtestutils.cpp)
add_test(NAME Test.KDED.TabletBackend COMMAND Test.KDED.TabletBackend)
ecm_mark_as_test(Test.KDED.TabletBackend)
target_link_libraries(Test.KDED.TabletBackend ${WACOM_KDED_TEST_LIBS})
