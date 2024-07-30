ecm_add_qml_module(bluetoothplugin URI org.kde.lingmo.private.bluetooth GENERATE_PLUGIN_SOURCE)

# Reuse applet's domain, because the only strings here so far were moved from it
target_compile_definitions(bluetoothplugin PRIVATE TRANSLATION_DOMAIN="lingmo_applet_org.kde.lingmo.bluetooth")

target_sources(bluetoothplugin PRIVATE
    devicesproxymodel.cpp
    devicesproxymodel.h
    devicesstateproxymodel.cpp
    devicesstateproxymodel.h
    launchapp.cpp
    launchapp.h
    utils.cpp
    utils.h
)

set_source_files_properties(
    SharedDevicesStateProxyModel.qml

    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

ecm_target_qml_sources(bluetoothplugin SOURCES
    SharedDevicesStateProxyModel.qml
)

target_link_libraries(bluetoothplugin PRIVATE
    Qt::Core
    Qt::Qml

    KF6::BluezQt
    KF6::I18n
    KF6::JobWidgets
    KF6::KIOGui
    KF6::Notifications
    KF6::Service
)

ecm_finalize_qml_module(bluetoothplugin)
