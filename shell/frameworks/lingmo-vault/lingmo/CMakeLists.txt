set(lingmovault_SRCS

    vaultapplet.cpp
    vaultapplet.h
    vaultsmodel.cpp
    vaultsmodel.h
    vaultsmodel_p.h

    ../common/vaultinfo.cpp
    ../common/vaultinfo.h
)

set_source_files_properties(
    ../common/org.kde.lingmovault.xml
    PROPERTIES
        INCLUDE ../common/vaultinfo.h
)

qt_add_dbus_interface(lingmovault_SRCS
    ../common/org.kde.lingmovault.xml
    lingmovault_interface
)

add_library(org.kde.lingmo.vault MODULE
    ${lingmovault_SRCS}
)

target_link_libraries(org.kde.lingmo.vault
    Qt::DBus
    Qt::Gui

    KF6::I18n

    Lingmo::Activities
    Lingmo::Lingmo
)

install(
    TARGETS org.kde.lingmo.vault
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/applets
)

lingmo_install_package(package org.kde.lingmo.vault)
