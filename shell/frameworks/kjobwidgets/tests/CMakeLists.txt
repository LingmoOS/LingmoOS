# tests need dbus due to KUiServerJobTracker
if (HAVE_QTDBUS)
  include(ECMMarkAsTest)

  find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

  macro(kjobwidgets_executable_tests)
    foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} Qt6::Test KF6::JobWidgets KF6::WidgetsAddons)
      ecm_mark_as_test(${_testname})
    endforeach(_testname)
  endmacro()

  kjobwidgets_executable_tests(kjobtrackerstest)

  set(kjobcreator_SRCS kjobcreator.cpp)
  qt_wrap_ui(kjobcreator_SRCS kjobcreator.ui)
  add_executable(kjobcreator ${kjobcreator_SRCS})
  target_link_libraries(kjobcreator Qt6::Widgets KF6::JobWidgets)
  ecm_mark_as_test(kjobcreator)
endif()
