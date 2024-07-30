find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED)
set_package_properties(Qt6Test PROPERTIES PURPOSE "Required for tests")

include(ECMMarkAsTest)

add_library(parts_test_interface INTERFACE)
target_link_libraries(parts_test_interface INTERFACE
    Qt6::Test KF6::Parts KF6::CoreAddons KF6::I18n KF6::XmlGui KF6::KIOCore
)

add_executable(kpartstest testmainwindow.cpp parts.cpp)
ecm_mark_as_test(kpartstest)
target_link_libraries(kpartstest parts_test_interface)

add_executable(normalktmtest normalktm.cpp parts.cpp)
ecm_mark_as_test(normalktmtest)
target_link_libraries(normalktmtest parts_test_interface)

########### unit tests ###############

MACRO(KPARTS_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname}  ${_testname}.cpp)
               ecm_mark_as_test(${_testname})
               target_link_libraries(${_testname} parts_test_interface)
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_EXECUTABLE_TESTS)

KPARTS_EXECUTABLE_TESTS(
    partviewer
    terminal_test
)
