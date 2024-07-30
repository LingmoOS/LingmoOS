remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

macro(KDBUSADDONS_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} KF6::DBusAddons)
    endforeach()
endmacro()

kdbusaddons_tests(
   dbuscalltest
   syncdbusenvtest
)

