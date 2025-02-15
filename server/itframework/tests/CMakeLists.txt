
include(ECMMarkAsTest)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(xmlgui_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt${QT_MAJOR_VERSION}::Test KF5::CoreAddons KF5::WidgetsAddons KF5::I18n KF5::XmlGui)
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
find_package(KF5TextWidgets QUIET)
if(TARGET KF5::TextWidgets)
    add_subdirectory(krichtexteditor)
endif()
