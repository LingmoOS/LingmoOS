// Copyright (C) 2024 Wenhao Peng <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class AccountPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Account")

public:
    explicit AccountPortal(QObject *parent);
    ~AccountPortal() = default;

public slots:
    uint GetUserInformation(const QDBusObjectPath &handle,
                            const QString &app_id,
                            const QString &window,
                            const QVariantMap &options,
                            QVariantMap &results);
};
