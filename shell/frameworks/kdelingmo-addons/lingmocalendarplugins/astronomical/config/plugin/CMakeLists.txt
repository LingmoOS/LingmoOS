ecm_add_qml_module(lingmocalendarastronomicaleventsconfig URI org.kde.lingmocalendar.astronomicaleventsconfig)
target_sources(lingmocalendarastronomicaleventsconfig PRIVATE configplugin.cpp configstorage.cpp)
target_link_libraries(lingmocalendarastronomicaleventsconfig PRIVATE
    Qt::Qml
    Qt::Core
    KF6::ConfigCore
)
ecm_finalize_qml_module(lingmocalendarastronomicaleventsconfig)
