// Copyright (C) 2024 pengwenhao <pengwenhao@deepin.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class AccessPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Access")

public:
    explicit AccessPortal(QObject *parent);
    ~AccessPortal() = default;

public slots:
    uint AccessDialog(const QDBusObjectPath &handle,
                      const QString &app_id,
                      const QString &parent_window,
                      const QString &title,
                      const QString &subtitle,
                      const QString &body,
                      const QVariantMap &options,
                      QVariantMap &results);
};
