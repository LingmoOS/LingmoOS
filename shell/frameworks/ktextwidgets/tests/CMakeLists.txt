include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(ktextwidgets_executable_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} Qt6::Test KF6::TextWidgets)
      ecm_mark_nongui_executable(${_testname})
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

ktextwidgets_executable_tests(
  ktextedittest
)
