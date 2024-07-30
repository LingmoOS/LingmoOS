
include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

########### unittests ###############

ecm_add_tests(
 test_filter.cpp
 test_core.cpp
 test_suggest.cpp
 test_settings.cpp
 test_autodetect.cpp
 NAME_PREFIX "sonnet-"
 LINK_LIBRARIES KF6::SonnetCore Qt6::Test
)

ecm_add_tests(
 test_highlighter.cpp
 NAME_PREFIX "sonnet-"
 LINK_LIBRARIES KF6::SonnetUi KF6::SonnetCore Qt6::Test
)
