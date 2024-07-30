add_executable(Test.Common.Enum testenum.cpp)
add_test(NAME Test.Common.Enum COMMAND Test.Common.Enum)
ecm_mark_as_test(Test.Common.Enum)
target_link_libraries(Test.Common.Enum ${WACOM_COMMON_TEST_LIBS})

