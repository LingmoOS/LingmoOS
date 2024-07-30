remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(proxymodeltestapp)

macro(kitemmodels_widgets_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname}
            KF6::ItemModels
            Qt6::Widgets
        )
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro(kitemmodels_widgets_tests)

kitemmodels_widgets_tests(
    rearrangecolumns
)
