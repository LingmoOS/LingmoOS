set(plasmavault_SRCS

    vaultapplet.cpp
    vaultapplet.h
    vaultsmodel.cpp
    vaultsmodel.h
    vaultsmodel_p.h

    ../common/vaultinfo.cpp
    ../common/vaultinfo.h
)

set_source_files_properties(
    ../common/org.kde.plasmavault.xml
    PROPERTIES
        INCLUDE ../common/vaultinfo.h
)

qt_add_dbus_interface(plasmavault_SRCS
    ../common/org.kde.plasmavault.xml
    plasmavault_interface
)

add_library(org.kde.plasma.vault MODULE
    ${plasmavault_SRCS}
)

target_link_libraries(org.kde.plasma.vault
    Qt::DBus
    Qt::Gui

    KF6::I18n

    Plasma::Activities
    Plasma::Plasma
)

install(
    TARGETS org.kde.plasma.vault
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets
)

plasma_install_package(package org.kde.plasma.vault)
