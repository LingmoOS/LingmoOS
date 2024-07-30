include(ECMMarkAsTest)

macro(bluezqt_executable_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} Qt6::DBus Qt6::Network Qt6::Test KF6BluezQt)
        ecm_mark_as_test(${_testname})
    endforeach(_testname)
endmacro()

bluezqt_executable_tests(
    adaptersreceiver
    devicereceiver
    chatprofile
    leserver
    mediaendpointconnector
)
