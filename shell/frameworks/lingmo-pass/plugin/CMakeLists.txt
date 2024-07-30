# SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

###### STATIC LIBRARY ######

set(lingmopasslib_SRCS
    abbreviations.cpp
    klipperutils.cpp
    otpprovider.cpp
    providerbase.cpp
    passwordfiltermodel.cpp
    passwordsmodel.cpp
    passwordsortproxymodel.cpp
    passwordprovider.cpp

    abbreviations.h
    klipperutils.h
    otpprovider.h
    providerbase.h
    passwordfiltermodel.h
    passwordsmodel.h
    passwordsortproxymodel.h
    passwordprovider.h
)

qt_add_dbus_interfaces(lingmopasslib_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.kde.klipper.klipper.xml
)

ecm_qt_declare_logging_category(lingmopasslib_SRCS
    HEADER lingmopass_debug.h
    IDENTIFIER LingmoPass::LINGMOPASS_LOG
    CATEGORY_NAME org.kde.lingmo.pass
)

add_library(lingmopass STATIC ${lingmopasslib_SRCS})
set_property(TARGET lingmopass PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(lingmopass
    Qt::Core
    Qt::DBus
    Qt::Qml
    Qt::Concurrent
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::ItemModels
    OATH::OATH
)

if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(lingmopass QGpgmeQt6 Lingmo::Lingmo5Support Qt::Gui KF6::ConfigCore KF6::CoreAddons)
else()
    target_link_libraries(lingmopass QGpgme KF${KF_MAJOR_VERSION}::Lingmo)
endif()


########### PLUGN ###########

set(lingmopassplugin_SRCS
    lingmopassplugin.cpp
    lingmopassplugin.h
)

add_library(lingmopassplugin SHARED ${lingmopassplugin_SRCS})
target_link_libraries(lingmopassplugin
    lingmopass
)


install(TARGETS lingmopassplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/lingmopass)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/lingmopass)
