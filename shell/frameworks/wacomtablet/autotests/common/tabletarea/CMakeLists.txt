add_executable(Test.Common.TabletArea testtabletarea.cpp)
add_test(NAME Test.Common.TabletArea COMMAND Test.Common.TabletArea)
ecm_mark_as_test(Test.Common.TabletArea)
target_link_libraries(Test.Common.TabletArea ${WACOM_COMMON_TEST_LIBS})
