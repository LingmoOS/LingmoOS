add_executable(Test.Common.TabletProfile testtabletprofile.cpp ../commontestutils.cpp)
add_test(NAME Test.Common.TabletProfile COMMAND Test.Common.TabletProfile)
ecm_mark_as_test(Test.Common.TabletProfile)
target_link_libraries(Test.Common.TabletProfile ${WACOM_COMMON_TEST_LIBS})
