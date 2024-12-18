// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "background.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopOCEANBackground, "xdg-ocean-background")

BackgroundPortal::BackgroundPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

bool BackgroundPortal::EnableAutostart(const QString &app_id,
                                       const bool &enable,
                                       const QStringList commandline,
                                       const uint &flags)
{
    qCDebug(XdgDesktopOCEANBackground) << "request autostart";
    return false;
}

uint BackgroundPortal::NotifyBackground(const QDBusObjectPath &handle, const QString &app_id, QString name, QVariantMap &results)
{
    qCDebug(XdgDesktopOCEANBackground) << "NotifyBackground";
    return 1;
}

QVariantMap BackgroundPortal::GetAppState()
{
    qCDebug(XdgDesktopOCEANBackground) << "Get AppState";
    return QVariantMap();
}
