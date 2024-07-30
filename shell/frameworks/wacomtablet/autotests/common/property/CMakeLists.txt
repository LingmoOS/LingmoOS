add_executable(Test.Common.Property testproperty.cpp)
add_test(NAME Test.Common.Property COMMAND Test.Common.Property)
ecm_mark_as_test(Test.Common.Property)
target_link_libraries(Test.Common.Property ${WACOM_COMMON_TEST_LIBS})
