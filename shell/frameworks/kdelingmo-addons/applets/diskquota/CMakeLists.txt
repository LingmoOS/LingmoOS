
#######################################################################################
# Package
lingmo_install_package(package org.kde.lingmo.diskquota)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="lingmo_applet_org.kde.lingmo.diskquota")

ecm_add_qml_module(diskquotaplugin URI org.kde.lingmo.private.diskquota)
target_sources(diskquotaplugin PRIVATE
    plugin/plugin.cpp
    plugin/DiskQuota.cpp
    plugin/QuotaListModel.cpp
    plugin/QuotaItem.cpp
)
target_link_libraries(diskquotaplugin PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::I18n
)
ecm_finalize_qml_module(diskquotaplugin)
