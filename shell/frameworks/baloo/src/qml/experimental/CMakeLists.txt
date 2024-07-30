add_definitions(-DTRANSLATION_DOMAIN=\"baloomonitorplugin\")

ecm_add_qml_module(baloomonitorplugin URI "org.kde.baloo.experimental" VERSION 0.1 GENERATE_PLUGIN_SOURCE)

target_sources(baloomonitorplugin PRIVATE
    monitor.cpp
)

target_link_libraries(baloomonitorplugin
    PUBLIC
    Qt6::Core
    Qt6::Qml
    Qt6::DBus
    KF6::I18n
    KF6::CoreAddons
    PRIVATE
    KF6::Baloo
    KF6::BalooEngine
    BalooDBusSchedulerInterface
    BalooDBusFileIndexerInterface
)

ecm_finalize_qml_module(baloomonitorplugin)
