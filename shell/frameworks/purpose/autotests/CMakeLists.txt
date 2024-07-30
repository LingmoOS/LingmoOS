ecm_add_test(alternativesmodeltest.cpp
    TEST_NAME alternativesmodeltest
    LINK_LIBRARIES Qt6::Test KF6::Purpose KF6::ConfigCore)
ecm_add_test(menutest.cpp
    TEST_NAME menutest
    LINK_LIBRARIES Qt6::Test KF6::PurposeWidgets)

set_property(TEST menutest PROPERTY ENVIRONMENT KDE_PURPOSE_LOCAL_JOBS=1 APPEND)
set_property(TEST alternativesmodeltest PROPERTY ENVIRONMENT KDE_PURPOSE_LOCAL_JOBS=1 APPEND)
