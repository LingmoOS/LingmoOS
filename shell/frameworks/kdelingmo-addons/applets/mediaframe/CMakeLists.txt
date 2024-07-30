lingmo_install_package(package org.kde.lingmo.mediaframe)

ecm_add_qml_module(mediaframeplugin URI org.kde.lingmo.private.mediaframe)
target_sources(mediaframeplugin PRIVATE
    plugin/mediaframe.cpp
    plugin/mediaframeplugin.cpp
)
target_link_libraries(mediaframeplugin PRIVATE
    Qt::Core
    Qt::Qml
    Qt::Quick
    KF6::I18n
    KF6::KIOCore
)
ecm_finalize_qml_module(mediaframeplugin)
