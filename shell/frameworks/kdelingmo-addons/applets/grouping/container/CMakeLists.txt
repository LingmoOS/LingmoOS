lingmo_install_package(package org.kde.lingmo.grouping)

set(groupedappletscontainer_SRCS
    groupedappletscontainer.cpp
)
ecm_qt_declare_logging_category(groupedappletscontainer_SRCS
    HEADER debug.h
    IDENTIFIER GROUPING_DEBUG
    CATEGORY_NAME kde.grouping
    DEFAULT_SEVERITY Info
    DESCRIPTION "Applet Grouping Container"
    EXPORT KDELINGMOADDONS
)

kcoreaddons_add_plugin(org.kde.lingmo.grouping SOURCES ${groupedappletscontainer_SRCS} INSTALL_NAMESPACE "lingmo/applets")
target_link_libraries(org.kde.lingmo.grouping
    Qt::Gui
    Qt::Quick
    Lingmo::Lingmo
    Lingmo::LingmoQuick
    KF6::XmlGui
    KF6::I18n
)
