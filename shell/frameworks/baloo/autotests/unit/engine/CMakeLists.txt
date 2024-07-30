MACRO(BALOO_ENGINE_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    ecm_add_test(${_testname}.cpp dbtest.cpp
        TEST_NAME ${_testname}
        LINK_LIBRARIES Qt6::Test KF6::BalooEngine
    )
  ENDFOREACH()
ENDMACRO()

baloo_engine_auto_tests(
    positiondbtest
    postingdbtest
    documentdbtest
    documenturldbtest
    documentiddbtest
    documentdatadbtest
    documenttimedbtest
    idtreedbtest
    idfilenamedbtest
    mtimedbtest

    termgeneratortest

    # Query
    andpostingiteratortest
    orpostingiteratortest
    postingiteratortest
    phraseanditeratortest
)
