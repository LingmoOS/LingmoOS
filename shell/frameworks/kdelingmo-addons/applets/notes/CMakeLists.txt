add_definitions(-DTRANSLATION_DOMAIN="lingmo_applet_org.kde.lingmo.notes")

lingmo_install_package(package org.kde.lingmo.notes)

ecm_add_qml_module(notesplugin URI org.kde.lingmo.private.notes)
target_sources(notesplugin PRIVATE
    plugin/abstractnoteloader.cpp
    plugin/documenthandler.cpp
    plugin/filesystemnoteloader.cpp
    plugin/note.cpp
    plugin/notemanager.cpp
    plugin/notesplugin.cpp
)
target_link_libraries(notesplugin PRIVATE
    Qt::Quick
    KF6::CoreAddons
)
ecm_finalize_qml_module(notesplugin)
