/*
    SPDX-FileCopyrightText: 2021 Méven Car <meven.car@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "GraphicsPlatformEntry.h"
#include <QGuiApplication>

GraphicsPlatformEntry::GraphicsPlatformEntry()
    : Entry(ki18n("Graphics Platform:"), graphicsPlatform())
{
}

QString GraphicsPlatformEntry::graphicsPlatform()
{
    const QString platformName = QGuiApplication::platformName();
    if (platformName.startsWith(QLatin1String("xcb"), Qt::CaseInsensitive)) {
        return QStringLiteral("X11");
    }
    if (platformName.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return QStringLiteral("Wayland");
    }
    return platformName;
}
