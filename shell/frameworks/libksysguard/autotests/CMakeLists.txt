find_package(Qt6 REQUIRED CONFIG COMPONENTS Test)

include_directories(${libksysguard_SOURCE_DIR})

# Process unit test
ecm_qt_declare_logging_category(processtest_debug_SRCS HEADER processcore_debug.h IDENTIFIER LIBKSYSGUARD_PROCESSCORE CATEGORY_NAME org.kde.libksysguard.processcore)
ecm_add_test(processtest.cpp processtest.h ${processtest_debug_SRCS} TEST_NAME processtest LINK_LIBRARIES KSysGuard::ProcessCore Qt::Test)

ecm_add_test(formattertest.cpp LINK_LIBRARIES Qt::Test KSysGuard::Formatter)
ecm_add_test(sensortreemodeltest.cpp LINK_LIBRARIES Qt::Test Qt::DBus KSysGuard::Sensors)
ecm_add_test(sensordatamodeltest.cpp LINK_LIBRARIES Qt::Test Qt::DBus KSysGuard::Sensors KSysGuard::Formatter)
ecm_add_test(processdatamodeltest.cpp LINK_LIBRARIES Qt::Test KSysGuard::ProcessCore)
ecm_add_test(cgrouptest.cpp LINK_LIBRARIES Qt::Test KF6::Service KSysGuard::ProcessCore)

include_directories(../faces)
ecm_add_test(sensorfacecontrollertest.cpp LINK_LIBRARIES
    Qt::Test
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::I18n
    KF6::Package
    KF6::ConfigQml
    KF6::Solid
    KSysGuard::SensorFaces
)
