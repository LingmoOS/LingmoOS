include(ECMAddTests)
find_package(Qt6Test REQUIRED)
configure_file(config-kdesudtest.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdesudtest.h)
ecm_add_test(kdesudtest.cpp ../lexer.cpp TEST_NAME kdesudtest LINK_LIBRARIES Qt6::Test KF6::CoreAddons KF6::ConfigCore)
