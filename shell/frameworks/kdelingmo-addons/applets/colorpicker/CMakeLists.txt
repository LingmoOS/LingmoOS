lingmo_install_package(package org.kde.lingmo.colorpicker)

ecm_add_qml_module(colorpickerplugin URI org.kde.lingmo.private.colorpicker)

target_sources(colorpickerplugin PRIVATE
    plugin/grabwidget.cpp
    plugin/colorpickerplugin.cpp
)
target_link_libraries(colorpickerplugin PRIVATE Qt::DBus Qt::Gui Qt::Qml Qt::Widgets)
ecm_finalize_qml_module(colorpickerplugin)
