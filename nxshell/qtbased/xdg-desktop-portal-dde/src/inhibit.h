// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class InhibitPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Inhibit")
    Q_PROPERTY(uint version READ version CONSTANT)
    inline uint version() const { return 3; }

public:
    explicit InhibitPortal(QObject *parent);
    ~InhibitPortal() = default;

public slots:
    void
    Inhibit(const QDBusObjectPath &handle, const QString &app_id, const QString &window, uint flags, const QVariantMap &options);

private slots:
    void onCloseRequested(const QVariant &data);
};
