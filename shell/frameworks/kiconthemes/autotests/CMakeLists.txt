
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include(ECMAddTests)

macro(KICONTHEMES_TESTS)
    foreach(_testname ${ARGN})
        set(_srcs ${_testname}.cpp)
        qt_add_resources(_srcs resources.qrc)
        ecm_add_test(${_srcs} TEST_NAME ${_testname} LINK_LIBRARIES Qt6::Test KF6::IconThemes KF6::WidgetsAddons KF6::ConfigCore KF6::IconWidgets)
    endforeach(_testname)
endmacro()

kiconthemes_tests(
  kiconloader_unittest
  kiconloader_resourcethemetest
  kicontheme_unittest
  kiconengine_unittest
  kiconengine_scaled_unittest
  kicondialog_unittest
  kpixmapsequenceloadertest
)

find_package(Qt6 COMPONENTS Quick)
if (TARGET Qt6::Quick)
  kiconthemes_tests(kquickiconprovidertest)
  target_link_libraries(kquickiconprovidertest Qt6::Quick)
endif()

set_tests_properties(kiconloader_unittest PROPERTIES RUN_SERIAL TRUE)

# Benchmark, compiled, but not run automatically with ctest
add_executable(kiconloader_benchmark kiconloader_benchmark.cpp)
target_link_libraries(kiconloader_benchmark Qt6::Test KF6::IconThemes KF6::WidgetsAddons KF6::ConfigCore)
