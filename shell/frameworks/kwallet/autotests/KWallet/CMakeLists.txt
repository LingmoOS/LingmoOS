remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include(ECMMarkAsTest)

macro(KWALLET_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(NAME itemviews-${_testname} COMMAND ${_testname})
        ecm_mark_as_test(${_testname})
        add_test(${_testname} ${_testname})
        target_link_libraries(${_testname} Qt6::Test KF6::Wallet)
    endforeach(_testname)
endmacro()

kwallet_tests(
)
