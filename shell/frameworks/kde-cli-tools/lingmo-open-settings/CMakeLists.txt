#  SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
#  SPDX-License-Identifier: GPL-2.0-or-later

add_executable(lingmo-open-settings main.cpp)
target_compile_definitions(lingmo-open-settings PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(lingmo-open-settings Qt::Core KF6::CoreAddons KF6::KIOGui KF6::I18n)
install(TARGETS lingmo-open-settings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.lingmo.settings.open.desktop DESTINATION  ${KDE_INSTALL_APPDIR})
