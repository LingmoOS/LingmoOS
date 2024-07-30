add_executable(Test.Common.ButtonShortcut testbuttonshortcut.cpp)
ecm_mark_as_test(Test.Common.ButtonShortcut)
add_test(NAME Test.Common.ButtonShortcut COMMAND Test.Common.ButtonShortcut)
target_link_libraries(Test.Common.ButtonShortcut ${WACOM_COMMON_TEST_LIBS})
