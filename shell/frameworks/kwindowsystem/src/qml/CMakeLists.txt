ecm_add_qml_module(KWindowSystem URI "org.kde.kwindowsystem" VERSION 1.0 GENERATE_PLUGIN_SOURCE)

target_sources(KWindowSystem PRIVATE types.h)
target_link_libraries(KWindowSystem PRIVATE Qt::Qml KF6::WindowSystem)

ecm_finalize_qml_module(KWindowSystem DESTINATION ${KDE_INSTALL_QMLDIR})
