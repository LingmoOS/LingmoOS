remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(kitemviews_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt6::Test KF6::ItemViews)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kitemviews_executable_tests(
  kcategorizedviewtest
  kwidgetitemdelegatetest
  ktreewidgetsearchlinetest
)
