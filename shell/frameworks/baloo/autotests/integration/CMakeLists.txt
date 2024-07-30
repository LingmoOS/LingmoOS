MACRO(BALOO_ENGINE_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp
        TEST_NAME ${_testname}
        LINK_LIBRARIES Qt6::Test KF6::BalooEngine
    )
  ENDFOREACH()
ENDMACRO()

baloo_engine_auto_tests(
    querytest
    writetransactiontest
)
target_link_libraries(querytest
    KF6::Baloo
)

add_executable(basicindexingbenchmark_bin basicindexingbenchmark.cpp)
target_link_libraries(basicindexingbenchmark_bin
    Qt6::Core
    KF6::BalooEngine
    baloofilecommon
)
ecm_mark_as_test(basicindexingbenchmark_bin)
ecm_mark_nongui_executable(basicindexingbenchmark_bin)

add_test(NAME basicindexingbenchmark_ts100 COMMAND basicindexingbenchmark_bin "-s" "100" ${CMAKE_SOURCE_DIR}/src)
add_test(NAME basicindexingbenchmark_ts50  COMMAND basicindexingbenchmark_bin "-s" "50"  ${CMAKE_SOURCE_DIR}/src)
