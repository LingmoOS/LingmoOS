add_library(ksysguard_sensorface MODULE sensorfacepackage.cpp)

target_link_libraries(ksysguard_sensorface PRIVATE KF6::Package KF6::I18n)

install(TARGETS ksysguard_sensorface DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/packagestructure)
