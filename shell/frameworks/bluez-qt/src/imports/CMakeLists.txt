ecm_add_qml_module(bluezqtextensionplugin URI "org.kde.bluezqt" CLASSNAME BluezQtExtensionPlugin VERSION 1.0 DEPENDENCIES QtCore)

target_sources(bluezqtextensionplugin PRIVATE
    declarativemanager.cpp
    declarativeadapter.cpp
    declarativebattery.cpp
    declarativedevice.cpp
    declarativeinput.cpp
    declarativemediaplayer.cpp
    declarativedevicesmodel.cpp
    bluezqtextensionplugin.cpp
)

ecm_target_qml_sources(bluezqtextensionplugin SOURCES DevicesModel.qml)

target_link_libraries(bluezqtextensionplugin PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::DBus
    KF6::BluezQt
)

ecm_finalize_qml_module(bluezqtextensionplugin DESTINATION ${KDE_INSTALL_QMLDIR})
