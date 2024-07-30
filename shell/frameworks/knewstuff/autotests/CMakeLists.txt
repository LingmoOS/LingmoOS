# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED)
set_package_properties(Qt6Test PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for unit tests")

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../src ../src )

add_library(knewstuff_test_static STATIC ${CMAKE_BINARY_DIR}/src/core/knewstuffcore_debug.cpp)
target_link_libraries(knewstuff_test_static PUBLIC Qt6::Xml Qt6::Test Qt6::Gui KF6::I18n KF6::NewStuffCore KF6::ConfigCore KF6::Archive KF6::Package knscore_jobs_static)
macro(knewstuff_unit_tests)
    foreach(_testmain ${ARGN})
       get_filename_component(_testname ${_testmain} NAME_WE)
       ecm_add_test(${_testmain}
                    TEST_NAME ${_testname}
                    NAME_PREFIX "knewstuff-")
       target_compile_definitions(${_testname} PUBLIC -DDATA_DIR="${CMAKE_CURRENT_BINARY_DIR}/data/")
       target_link_libraries(${_testname} knewstuff_test_static)
    endforeach()
endmacro()

knewstuff_unit_tests(
    knewstuffentrytest.cpp
)

add_subdirectory(core)
