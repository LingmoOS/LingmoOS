set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test)

include_directories(../src)
aux_source_directory(. charcode_test_src)
add_executable(charcode_test ${charcode_test_src})
target_link_libraries(charcode_test
  ${charcode_library}
  ${charcode_deps}
  ${GTEST_LIBRARIES})
if(BUILD_SHARED_LIBS)
  target_compile_definitions(charcode_test PRIVATE RIME_IMPORTS)
endif(BUILD_SHARED_LIBS)

set(charcode_test_executable ${EXECUTABLE_OUTPUT_PATH}/charcode_test)
add_test(charcode_test ${charcode_test_executable})
