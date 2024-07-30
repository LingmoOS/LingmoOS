include(ECMAddTests)

########### unittests ###############

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Concurrent Test)

macro(KSERVICE_UNIT_TESTS)
  foreach(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp TEST_NAME ${_testname}
        LINK_LIBRARIES KF6::Service Qt6::Test Qt6::Concurrent KF6::ConfigCore KF6::CoreAddons Qt6::Xml)
    target_compile_definitions(${_testname} PRIVATE -DKBUILDSYCOCAEXE=\"$<TARGET_FILE:kbuildsycoca6>\")
  endforeach()
endmacro(KSERVICE_UNIT_TESTS)

kservice_unit_tests(
 ksycocatest
 ksycoca_xdgdirstest
 ksycocathreadtest
 kservicetest
 kapplicationtradertest
)

# the test plays with the timestamp of ~/.qttest/share/applications, and with the ksycoca file, other tests can collide
set_tests_properties(ksycocatest PROPERTIES RUN_SERIAL TRUE)
# KServiceTest::testAllServices can fail if any service is deleted while the test runs
set_tests_properties(kservicetest PROPERTIES RUN_SERIAL TRUE)

######### kmimeassociationstest ########

set(kmimeassociationstest_SRCS kmimeassociationstest.cpp ../src/sycoca/kmimeassociations.cpp)
ecm_qt_declare_logging_category(kmimeassociationstest_SRCS
    HEADER sycocadebug.h
    IDENTIFIER SYCOCA
    CATEGORY_NAME kf.service.sycoca
)

ecm_add_test(${kmimeassociationstest_SRCS} TEST_NAME kmimeassociationstest
             LINK_LIBRARIES KF6::Service Qt6::Test Qt6::Xml KF6::ConfigCore KF6::CoreAddons)

