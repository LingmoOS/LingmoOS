# SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

ecm_qt_declare_logging_category(shared_debug_SRCS HEADER debug.h IDENTIFIER FALLBACK_SESSION_RESTORE CATEGORY_NAME org.kde.lingmo-fallback-session-restore)


add_executable(lingmo-fallback-session-save
    save.cpp
    ${shared_debug_SRCS}
)
target_link_libraries(lingmo-fallback-session-save
    Qt6::Core
    KF6::KIOGui
    KF6::ConfigCore
    PW::LibTaskManager
    Qt6::GuiPrivate
    Wayland::Client
)


add_executable( lingmo-fallback-session-restore
    restore.cpp
    ${shared_debug_SRCS}
)
kconfig_add_kcfg_files(lingmo-fallback-session-restore smserversettings.kcfgc)

target_link_libraries( lingmo-fallback-session-restore
    Qt6::Core
    KF6::KIOGui
    KF6::ConfigCore
    KF6::ConfigGui
)

install(TARGETS lingmo-fallback-session-restore DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})
install(TARGETS lingmo-fallback-session-save DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})

ecm_install_configured_files(INPUT org.kde.lingmo-fallback-session-save.desktop.in @ONLY DESTINATION ${KDE_INSTALL_APPDIR})
ecm_install_configured_files(INPUT org.kde.lingmo-fallback-session-restore.desktop.in @ONLY DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})
