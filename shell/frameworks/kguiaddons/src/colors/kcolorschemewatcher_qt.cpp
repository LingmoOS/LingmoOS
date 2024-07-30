/*
 * SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcolorschemewatcher_qt.h"

#include <QGuiApplication>
#include <QStyleHints>

KColorSchemeWatcherQt::KColorSchemeWatcherQt()
{
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &KColorSchemeWatcherBackend::systemPreferenceChanged);
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcherQt::systemPreference() const
{
    switch (QGuiApplication::styleHints()->colorScheme()) {
    case Qt::ColorScheme::Unknown:
        return KColorSchemeWatcher::NoPreference;
    case Qt::ColorScheme::Light:
        return KColorSchemeWatcher::PreferLight;
    case Qt::ColorScheme::Dark:
        return KColorSchemeWatcher::PreferDark;
    }
    return KColorSchemeWatcher::NoPreference;
}

#include "moc_kcolorschemewatcher_qt.cpp"
