find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt6Test PROPERTIES PURPOSE "Required for tests")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include(ECMMarkAsTest)
include(ECMAddTests)

MACRO(KSVG_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               set(libs Qt6::Qml Qt6::Test KF6::Svg
                        KF6::Archive KF6::CoreAddons KF6::ConfigGui)
               if(QT_QTOPENGL_FOUND)
                   list(APPEND libs Qt6::OpenGL)
               endif()
               ecm_add_test(${_testname}.cpp
                            LINK_LIBRARIES ${libs}
                            NAME_PREFIX "plasmasvg-")
               qt_add_resources(${_testname} "images"
                       FILES "data/background.svgz")
               target_include_directories(${_testname} PRIVATE )
       ENDFOREACH(_testname)
ENDMACRO(KSVG_UNIT_TESTS)

KSVG_UNIT_TESTS(
    framesvgtest
)

