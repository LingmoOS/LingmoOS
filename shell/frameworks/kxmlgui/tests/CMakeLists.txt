
include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(xmlgui_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt6::Test KF6::CoreAddons KF6::WidgetsAddons KF6::I18n KF6::XmlGui)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

xmlgui_executable_tests(
  kbugreporttest
  kmainwindowrestoretest
  kmainwindowtest
  krulertest
  ktoolbartest
  kxmlguitest
  kxmlguiwindowtest
  kwindowtest
)

# KTextWidgets dependency is only needed for this test
find_package(KF6TextWidgets QUIET)
if(TARGET KF6::TextWidgets)
    add_subdirectory(krichtexteditor)
endif()
