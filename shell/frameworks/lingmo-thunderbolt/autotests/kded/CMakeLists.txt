include(ECMAddTests)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/kded
    ${CMAKE_BINARY_DIR}/src/kded
)

ecm_add_test(
    kdedtest.cpp
    ${CMAKE_SOURCE_DIR}/src/kded/kded_bolt.cpp
    ${CMAKE_BINARY_DIR}/src/kded/kded_bolt_debug.cpp
    TEST_NAME kdedtest
    LINK_LIBRARIES kbolt libboltfakeserver Qt::Test Qt::Core Qt::DBus KF6::CoreAddons KF6::DBusAddons KF6::I18n KF6::Notifications
    NAME_PREFIX kbolt-kded-
)
set_target_properties(kdedtest PROPERTIES CXX_STANDARD 14)

