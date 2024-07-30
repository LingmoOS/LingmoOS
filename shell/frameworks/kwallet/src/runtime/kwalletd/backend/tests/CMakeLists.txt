set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_BINARY_DIR}/..
)

include(ECMMarkAsTest)

macro(kwallet_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname} Qt6::Widgets KF6WalletBackend)
  endforeach(_testname)
endmacro()

kwallet_executable_tests(
  backendtest
  testbf
  testsha
)
