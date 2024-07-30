#
# Query Serialization
#
ecm_add_test(queryserializationtest.cpp
    TEST_NAME "queryserializationtest"
    LINK_LIBRARIES Qt6::Test KF6::Baloo
)
ecm_add_test(filemonitortest.cpp
    TEST_NAME "filemonitortest"
    LINK_LIBRARIES Qt6::Test Qt6::DBus KF6::Baloo
)

#
# Advanced Query Parser
#
ecm_add_test(advancedqueryparsertest.cpp ../../../src/lib/advancedqueryparser.cpp ../../../src/lib/term.cpp
    TEST_NAME "advancedqueryparsertest"
    LINK_LIBRARIES Qt6::Test
)

#
# Fetch Job
#
set(filefetchjobtest_SRC filefetchjobtest.cpp)
ecm_add_test(${filefetchjobtest_SRC}
    TEST_NAME "filefetchjobtest"
    LINK_LIBRARIES Qt6::Test KF6::Baloo KF6::BalooEngine KF6::FileMetaData baloofilecommon
)
