include(ECMAddTests)
include(ECMMarkAsTest)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

macro(kconfigwidgets_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kconfigwidgets_executable_tests(
   kcolorschemedemo
   klanguagebuttontest
   kcommandbartest
)

if (TARGET Qt6::Core5Compat)
    set(kcodecactiontest_libs Qt6::Core5Compat)
endif()
add_executable(kcodecactiontest kcodecactiontest.cpp)
target_link_libraries(kcodecactiontest Qt${QT_MAJOR_VERSION}::Test KF5::ConfigWidgets ${kcodecactiontest_libs})
ecm_mark_as_test(kcodecactiontest)

## kcolorutilsdemo

set(kcolorUtilsDemoSources kcolorutilsdemo.cpp kimageframe.cpp)
qt_wrap_ui(kcolorUtilsDemoSources kcolorutilsdemo.ui)
add_executable(kcolorutilsdemo ${kcolorUtilsDemoSources})
ecm_mark_as_test(kcolorutilsdemo)
target_link_libraries(kcolorutilsdemo KF5::ConfigWidgets KF5::GuiAddons)

## krecentfilesactiontest (manual)

set(krecentfilesactionTestSources krecentfilesactiontest.cpp)
qt_wrap_ui(krecentfilesactionTestSources krecentfilesactiontest.ui)
add_executable(krecentfilesactiontest ${krecentfilesactionTestSources})
ecm_mark_as_test(krecentfilesactiontest)
target_link_libraries(krecentfilesactiontest KF5::ConfigWidgets)
