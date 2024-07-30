add_executable(Test.Common.ProfileManager testprofilemanager.cpp ../commontestutils.cpp)
add_test(NAME Test.Common.ProfileManager COMMAND Test.Common.ProfileManager)
ecm_mark_as_test(Test.Common.ProfileManager)
target_link_libraries(Test.Common.ProfileManager ${WACOM_COMMON_TEST_LIBS})

