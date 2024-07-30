plasma_install_package(package org.kde.plasma.mediaframe)

ecm_add_qml_module(mediaframeplugin URI org.kde.plasma.private.mediaframe)
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
