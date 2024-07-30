remove_definitions(-DQT_NO_CAST_FROM_ASCII)

ecm_add_tests(sentryenvelopetest.cpp
        sentrypostboxtest.cpp
        sentrydsnstest.cpp
    LINK_LIBRARIES Qt::Core Qt::Test DrKonqiSentryInternal)
