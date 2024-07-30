add_executable(Test.Common.DeviceProperty testdeviceproperty.cpp)
add_test(NAME Test.Common.DeviceProperty COMMAND Test.Common.DeviceProperty)
ecm_mark_as_test(Test.Common.DeviceProperty)
target_link_libraries(Test.Common.DeviceProperty ${WACOM_COMMON_TEST_LIBS})
