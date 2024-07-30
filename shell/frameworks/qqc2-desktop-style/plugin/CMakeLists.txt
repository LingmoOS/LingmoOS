ecm_add_qml_module(qqc2desktopstyleplugin URI org.kde.qqc2desktopstyle.private GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)

target_sources(qqc2desktopstyleplugin PRIVATE
    kquickstyleitem.cpp
    itembranchindicators.cpp
)

target_link_libraries(qqc2desktopstyleplugin PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::Gui
    Qt6::Widgets
    KF6::ConfigCore
    KF6::KirigamiPlatform
)

ecm_finalize_qml_module(qqc2desktopstyleplugin)
