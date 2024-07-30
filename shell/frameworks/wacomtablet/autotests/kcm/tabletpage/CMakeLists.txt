add_executable(Test.KCM.TabletPage testtabletpage.cpp ../profilemanagementmocks.cpp)
add_test(NAME Test.KCM.TabletPage COMMAND Test.KCM.TabletPage)
ecm_mark_as_test(Test.KCM.TabletPage)
target_link_libraries(Test.KCM.TabletPage ${WACOM_KCM_TEST_LIBS})
