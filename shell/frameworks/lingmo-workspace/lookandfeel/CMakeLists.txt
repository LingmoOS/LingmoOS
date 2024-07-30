lingmo_install_package(org.kde.ocean org.kde.ocean.desktop look-and-feel lookandfeel)
lingmo_install_package(org.kde.oceandark org.kde.oceandark.desktop look-and-feel lookandfeel)
lingmo_install_package(org.kde.oceantwilight org.kde.oceantwilight.desktop look-and-feel lookandfeel)

ecm_add_qml_module(components URI "org.kde.ocean.components" NO_GENERATE_PLUGIN_SOURCE)
ecm_target_qml_sources(components SOURCES
    components/animation/RejectPasswordAnimation.qml
    components/animation/RejectPasswordPathAnimation.qml
    components/ActionButton.qml
    components/Battery.qml
    components/Clock.qml
    components/SessionManagementScreen.qml
    components/UserDelegate.qml
    components/UserList.qml
    components/VirtualKeyboard.qml
    components/VirtualKeyboard_wayland.qml
    components/VirtualKeyboardLoader.qml
    components/WallpaperFader.qml
)
target_sources(components PRIVATE components/componentsplugin.cpp)
ecm_finalize_qml_module(components DESTINATION ${KDE_INSTALL_QMLDIR})

qt_add_shaders(components LNFSHADERS
    PRECOMPILE
    BATCHABLE
    OPTIMIZED
    PREFIX "/qt/qml/org/kde/ocean/components/shaders/"
    FILES
        components/UserDelegate.frag
        components/WallpaperFader.frag
    OUTPUTS
        UserDelegate.frag.qsb
        WallpaperFader.frag.qsb
)
