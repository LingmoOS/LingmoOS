
add_executable(Test.KDED.XInputAdaptor testxinputadaptor.cpp ../tabletdependenttest.cpp)
add_test(NAME Test.KDED.XInputAdaptor COMMAND Test.KDED.XInputAdaptor)
ecm_mark_as_test(Test.KDED.XInputAdaptor)
target_link_libraries(Test.KDED.XInputAdaptor ${WACOM_KDED_TEST_LIBS})

