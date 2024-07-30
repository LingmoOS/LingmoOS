include(ECMAddTests)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

ecm_add_tests(
    valuetest.cpp
    categorytest.cpp
    convertertest.cpp
    LINK_LIBRARIES KF6::UnitConversion KF6::I18n Qt6::Test
)

qt_add_resources(CURRENCY_TEST_RESOURCES currencytableinittest/data.qrc)
ecm_add_test(
    currencytableinittest.cpp
    ${CURRENCY_TEST_RESOURCES}
    TEST_NAME currencytableinittest
    LINK_LIBRARIES KF6::UnitConversion KF6::I18n Qt6::Test
)
