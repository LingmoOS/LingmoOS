# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(kcalendarcoreqml URI "org.kde.calendarcore" CLASS_NAME KCalendarCoreQmlPlugin DEPENDENCIES QtCore)

target_sources(kcalendarcoreqml PRIVATE
    kcalendarcoreqmlplugin.cpp
    types.h
)

target_link_libraries(kcalendarcoreqml PRIVATE KF6CalendarCore Qt6::Qml)

ecm_finalize_qml_module(kcalendarcoreqml)
