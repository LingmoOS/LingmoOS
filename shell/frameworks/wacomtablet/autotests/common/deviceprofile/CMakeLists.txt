add_executable(Test.Common.DeviceProfile testdeviceprofile.cpp ../commontestutils.cpp)
add_test(NAME Test.Common.DeviceProfile COMMAND Test.Common.DeviceProfile)
ecm_mark_as_test(Test.Common.DeviceProfile)
target_link_libraries(Test.Common.DeviceProfile ${WACOM_COMMON_TEST_LIBS})
