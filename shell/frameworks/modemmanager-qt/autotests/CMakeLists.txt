include_directories(
    ${MODEMMANAGER_INCLUDE_DIRS}
)

include(ECMAddTests)

find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Test)
set_package_properties(Qt6Test PROPERTIES PURPOSE "Required for autotests")

macro(serialize_test target)
    set_property(TEST ${target} PROPERTY RUN_SERIAL 1)
endmacro()

ecm_add_test(bearerpropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(bearerpropertiestest)
ecm_add_test(modemtest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(modemtest)
ecm_add_test(modempropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(modempropertiestest)
ecm_add_test(modem3gpppropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(modem3gpppropertiestest)
ecm_add_test(modem3gppussdpropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(modem3gppussdpropertiestest)
ecm_add_test(modemcdmapropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(modemcdmapropertiestest)
ecm_add_test(modemmessagingpropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(modemmessagingpropertiestest)
ecm_add_test(modemlocationpropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(modemlocationpropertiestest)
ecm_add_test(modemtimepropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
serialize_test(modemtimepropertiestest)

if (${MODEMMANAGER_VERSION} VERSION_EQUAL 1.2.0 OR ${MODEMMANAGER_VERSION} VERSION_GREATER 1.2.0)
    ecm_add_test(modemomapropertiestest.cpp LINK_LIBRARIES Qt6::Test KF6ModemManagerQt_static fakeModem)
    serialize_test(modemomapropertiestest)
endif()
