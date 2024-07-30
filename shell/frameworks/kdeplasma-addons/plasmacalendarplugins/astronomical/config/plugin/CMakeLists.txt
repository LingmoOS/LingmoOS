ecm_add_qml_module(plasmacalendarastronomicaleventsconfig URI org.kde.plasmacalendar.astronomicaleventsconfig)
target_sources(plasmacalendarastronomicaleventsconfig PRIVATE configplugin.cpp configstorage.cpp)
target_link_libraries(plasmacalendarastronomicaleventsconfig PRIVATE
    Qt::Qml
    Qt::Core
    KF6::ConfigCore
)
ecm_finalize_qml_module(plasmacalendarastronomicaleventsconfig)
