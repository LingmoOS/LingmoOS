add_executable(Test.KDED.XsetWacomAdaptor testxsetwacomadaptor.cpp ../tabletdependenttest.cpp)
add_test(NAME Test.KDED.XsetWacomAdaptor COMMAND Test.KDED.XsetWacomAdaptor)
ecm_mark_as_test(Test.KDED.XsetWacomAdaptor)
target_link_libraries(Test.KDED.XsetWacomAdaptor ${WACOM_KDED_TEST_LIBS})
