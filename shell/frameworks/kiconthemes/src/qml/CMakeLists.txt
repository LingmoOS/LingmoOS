ecm_add_qml_module(iconthemesplugin URI "org.kde.iconthemes" VERSION 1.0 GENERATE_PLUGIN_SOURCE)
target_sources(iconthemesplugin PRIVATE icondialog.cpp)
target_link_libraries(iconthemesplugin PRIVATE Qt6::Qml Qt6::Quick KF6::IconWidgets)
ecm_finalize_qml_module(iconthemesplugin DESTINATION ${KDE_INSTALL_QMLDIR})

