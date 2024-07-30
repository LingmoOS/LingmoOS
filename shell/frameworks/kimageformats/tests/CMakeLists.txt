add_definitions(-DPLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}/../src")

include(ECMMarkAsTest)

macro(kimageformats_executable_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} Qt6::Gui)
        ecm_mark_as_test(${_testname})
    endforeach(_testname)
endmacro()

kimageformats_executable_tests(
    imageconverter
    imagedump
)
