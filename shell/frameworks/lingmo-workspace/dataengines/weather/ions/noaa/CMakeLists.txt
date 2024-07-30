set (ion_noaa_SRCS ion_noaa.cpp)
ecm_qt_declare_logging_category(ion_noaa_SRCS
    HEADER ion_noaadebug.h
    IDENTIFIER IONENGINE_NOAA
    CATEGORY_NAME kde.dataengine.ion.noaa
    DEFAULT_SEVERITY Info
    DESCRIPTION "DataEngine ion noaa"
    EXPORT LINGMOWORKSPACE
)
add_library(lingmo_engine_noaa MODULE ${ion_noaa_SRCS})
target_link_libraries (lingmo_engine_noaa
    weather_ion
    KF6::KIOCore
    KF6::UnitConversion
    KF6::I18n
)

install (TARGETS lingmo_engine_noaa DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo5support/dataengine)
install (FILES noaa_station_list.xml DESTINATION ${KDE_INSTALL_DATADIR}/lingmo/weather)
