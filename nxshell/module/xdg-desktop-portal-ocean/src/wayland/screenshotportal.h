// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "abstractwaylandportal.h"

#include <QDBusObjectPath>
#include <QObject>

class ScreenshotPortalWayland : public AbstractWaylandPortal
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Screenshot")

public:
    ScreenshotPortalWayland(PortalWaylandContext *context);

    QString fullScreenShot();
    QString captureInteractively();

public Q_SLOTS:
    uint PickColor(const QDBusObjectPath &handle,
                   const QString &app_id,
                   const QString &parent_window,
                   const QVariantMap &options,
                   QVariantMap &results);
    uint Screenshot(const QDBusObjectPath &handle,
                    const QString &app_id,
                    const QString &parent_window,
                    const QVariantMap &options,
                    QVariantMap &results);
};
