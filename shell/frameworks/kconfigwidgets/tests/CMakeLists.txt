include(ECMAddTests)
include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

macro(kconfigwidgets_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt6::Test KF6::ConfigWidgets)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kconfigwidgets_executable_tests(
   kcolorschemedemo
   klanguagebuttontest
   kcommandbartest
)

add_executable(kcodecactiontest kcodecactiontest.cpp)
target_link_libraries(kcodecactiontest Qt6::Test KF6::ConfigWidgets)
ecm_mark_as_test(kcodecactiontest)

## kcolorutilsdemo

set(kcolorUtilsDemoSources kcolorutilsdemo.cpp kimageframe.cpp)
qt_wrap_ui(kcolorUtilsDemoSources kcolorutilsdemo.ui)
add_executable(kcolorutilsdemo ${kcolorUtilsDemoSources})
ecm_mark_as_test(kcolorutilsdemo)
target_link_libraries(kcolorutilsdemo KF6::ConfigWidgets KF6::GuiAddons)

## krecentfilesactiontest (manual)

set(krecentfilesactionTestSources krecentfilesactiontest.cpp)
qt_wrap_ui(krecentfilesactionTestSources krecentfilesactiontest.ui)
add_executable(krecentfilesactiontest ${krecentfilesactionTestSources})
ecm_mark_as_test(krecentfilesactiontest)
target_link_libraries(krecentfilesactiontest KF6::ConfigWidgets)
