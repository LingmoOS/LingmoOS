add_executable(Test.KCM.StylusPage teststyluspage.cpp ../profilemanagementmocks.cpp)
add_test(NAME Test.KCM.StylusPage COMMAND Test.KCM.StylusPage)
ecm_mark_as_test(Test.KCM.StylusPage)
target_link_libraries(Test.KCM.StylusPage ${WACOM_KCM_TEST_LIBS})
