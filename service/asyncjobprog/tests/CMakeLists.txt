
include(ECMMarkAsTest)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(kjobwidgets_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt${QT_MAJOR_VERSION}::Test KF5::JobWidgets)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kjobwidgets_executable_tests(
   kjobtrackerstest
)

set(kjobcreator_SRCS kjobcreator.cpp)
qt_wrap_ui(kjobcreator_SRCS kjobcreator.ui)
add_executable(kjobcreator ${kjobcreator_SRCS})
target_link_libraries(kjobcreator Qt${QT_MAJOR_VERSION}::Widgets KF5::JobWidgets)
ecm_mark_as_test(kjobcreator)
