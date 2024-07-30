
include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(kiconthemes_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt6::Test KF6::IconThemes KF6::WidgetsAddons KF6::IconWidgets)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kiconthemes_executable_tests(
  kiconbuttontest
  kicondialogtest
  kiconloadertest
  kiconeffecttest
)
