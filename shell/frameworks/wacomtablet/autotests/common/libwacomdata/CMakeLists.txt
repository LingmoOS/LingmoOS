add_executable(Test.Common.LibwacomData testlibwacomdata.cpp)

# This is technically a test but it's very long, verbose and would fail most of the time
# so for now make it for humans only

#add_test(NAME Test.Common.LibwacomData COMMAND Test.Common.LibwacomData)
#ecm_mark_as_test(Test.Common.LibwacomData)
target_link_libraries(Test.Common.LibwacomData ${WACOM_COMMON_TEST_LIBS})
