add_executable(Test.Common.TabletInformation testtabletinformation.cpp ../commontestutils.cpp)
add_test(NAME Test.Common.TabletInformation COMMAND Test.Common.TabletInformation)
ecm_mark_as_test(Test.Common.TabletInformation)
target_link_libraries(Test.Common.TabletInformation ${WACOM_COMMON_TEST_LIBS})
