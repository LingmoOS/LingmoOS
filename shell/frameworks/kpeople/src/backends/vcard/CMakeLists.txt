# SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

kcoreaddons_add_plugin(KPeopleVCard SOURCES kpeoplevcard.cpp INSTALL_NAMESPACE "kpeople/datasource")
target_link_libraries(KPeopleVCard KF6::PeopleBackend KF6::CoreAddons KF6::Contacts KF6::I18n)
