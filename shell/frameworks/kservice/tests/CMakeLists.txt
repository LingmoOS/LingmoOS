include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(kservice_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} KF6::Service)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kservice_executable_tests(
  kservicegroup_dumper
  findservice
)

add_executable(kmimeassociations_dumper)
ecm_mark_as_test(kmimeassociations_dumper)

ecm_qt_declare_logging_category(kmimeassociations_dumper
    HEADER sycocadebug.h
    IDENTIFIER SYCOCA
    CATEGORY_NAME kf.service.sycoca
)
target_sources(kmimeassociations_dumper PRIVATE
    kmimeassociations_dumper.cpp
    ../src/sycoca/kmimeassociations.cpp
)

target_link_libraries(kmimeassociations_dumper KF6::Service KF6::CoreAddons KF6::ConfigCore)
