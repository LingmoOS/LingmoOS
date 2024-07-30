# SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

add_executable(lingmo6.0-remove-old-shortcuts)
target_sources(lingmo6.0-remove-old-shortcuts PRIVATE lingmo6.0-remove-old-shortcuts.cpp)
target_link_libraries(lingmo6.0-remove-old-shortcuts PRIVATE KF6::ConfigCore KF6::GlobalAccel)

install(FILES lingmo6.0-remove-old-shortcuts.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
install(TARGETS lingmo6.0-remove-old-shortcuts DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)

add_executable(lingmoshell-6.0-keep-default-floating-setting-for-lingmo-5-panels)
target_sources(lingmoshell-6.0-keep-default-floating-setting-for-lingmo-5-panels PRIVATE lingmoshell-6.0-keep-default-floating-setting-for-lingmo-5-panels.cpp)
target_link_libraries(lingmoshell-6.0-keep-default-floating-setting-for-lingmo-5-panels KF6::ConfigCore)

add_executable(lingmoshell-6.0-keep-custom-position-of-panels)
target_sources(lingmoshell-6.0-keep-custom-position-of-panels PRIVATE lingmoshell-6.0-keep-custom-position-of-panels.cpp)
target_link_libraries(lingmoshell-6.0-keep-custom-position-of-panels KF6::ConfigCore)

install(FILES lingmoshell-6.0-keep-default-floating-setting-for-lingmo-5-panels.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
install(TARGETS lingmoshell-6.0-keep-default-floating-setting-for-lingmo-5-panels DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)

install(FILES lingmoshell-6.0-keep-custom-position-of-panels.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
install(TARGETS lingmoshell-6.0-keep-custom-position-of-panels DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin/)
