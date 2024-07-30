set(ion_wettercom_SRCS ion_wettercom.cpp)
ecm_qt_declare_logging_category(ion_wettercom_SRCS
    HEADER ion_wettercomdebug.h
    IDENTIFIER IONENGINE_WETTERCOM
    CATEGORY_NAME kde.dataengine.ion.wettercom
    DEFAULT_SEVERITY Info
    DESCRIPTION "DataEngine ion wettercom"
    EXPORT LINGMOWORKSPACE
)
add_library(lingmo_engine_wettercom MODULE ${ion_wettercom_SRCS})
target_link_libraries(lingmo_engine_wettercom
    weather_ion
    KF6::KIOCore
    KF6::UnitConversion
    KF6::I18n
)

install(TARGETS lingmo_engine_wettercom DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo5support/dataengine)

