add_executable(Test.Common.DeviceInformation testdeviceinformation.cpp ../commontestutils.cpp)
add_test(NAME Test.Common.DeviceInformation COMMAND Test.Common.DeviceInformation)
ecm_mark_as_test(Test.Common.DeviceInformation)
target_link_libraries(Test.Common.DeviceInformation ${WACOM_COMMON_TEST_LIBS})
