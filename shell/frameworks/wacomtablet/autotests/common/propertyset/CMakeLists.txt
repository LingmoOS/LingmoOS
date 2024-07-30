add_executable(Test.Common.PropertySet testpropertyset.cpp)
add_test(NAME Test.Common.PropertySet COMMAND Test.Common.PropertySet)
ecm_mark_as_test(Test.Common.PropertySet)
target_link_libraries(Test.Common.PropertySet ${WACOM_COMMON_TEST_LIBS})
