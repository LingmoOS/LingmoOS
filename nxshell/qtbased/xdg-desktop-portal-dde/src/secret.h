// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusObjectPath>
#include <QDBusAbstractAdaptor>
#include <qdbusargument.h>

class SecretPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Secret")

    Q_PROPERTY(uint version READ version CONSTANT)
    inline uint version() const { return 1; }

public:
    SecretPortal(QObject *parent);
    ~SecretPortal() = default;

public slots:
    uint RetrieveSecret(const QDBusObjectPath &handle,
                        const QString &app_id,
                        const QDBusUnixFileDescriptor &fd,
                        const QVariantMap &options,
                        QVariantMap &result);
};
