add_executable(Test.Common.ScreenSpace testscreenspace.cpp)
add_test(NAME Test.Common.ScreenSpace COMMAND Test.Common.ScreenSpace)
ecm_mark_as_test(Test.Common.ScreenSpace)
target_link_libraries(Test.Common.ScreenSpace ${WACOM_COMMON_TEST_LIBS})
