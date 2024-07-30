# SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

kcoreaddons_add_plugin(kameleon
    SOURCES
        kameleon.cpp
        kameleon.h
    INSTALL_NAMESPACE
        "kf6/kded")

target_link_libraries(kameleon
                      KF6::CoreAddons
                      KF6::I18n
                      KF6::AuthCore
                      KF6::ConfigCore
                      KF6::ConfigGui
                      KF6::DBusAddons
                      KF6::WidgetsAddons)

ecm_qt_declare_logging_category(kameleon
                      HEADER kameleon_debug.h
                      IDENTIFIER KAMELEON
                      CATEGORY_NAME org.kde.kameleon
                      DESCRIPTION "kded module for accent coloring device leds"
                      EXPORT KDEPLASMAADDONS
                  )

########### kauth helper ################
add_executable(kameleonhelper)
target_sources(kameleonhelper PRIVATE kameleonhelper.cpp kameleonhelper.h)

target_link_libraries(kameleonhelper
    Qt6::Core
    KF6::AuthCore
    KF6::CoreAddons
)

install(TARGETS kameleonhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(kameleonhelper org.kde.kameleonhelper root)
kauth_install_actions(org.kde.kameleonhelper kameleonhelper.actions)

ecm_qt_declare_logging_category(kameleonhelper
                      HEADER kameleonhelper_debug.h
                      IDENTIFIER KAMELEONHELPER
                      CATEGORY_NAME org.kde.kameleonhelper
                      DESCRIPTION "Helper for kded module for accent coloring device leds"
                      EXPORT KDEPLASMAADDONS
                  )
