// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class GlobalShortcutPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.GlobalShortcuts")
    Q_PROPERTY(uint version READ version CONSTANT)
    inline uint version() const { return 1; }

public:
    explicit GlobalShortcutPortal(QObject *parent);
    ~GlobalShortcutPortal() = default;

public slots:
    uint CreateSession(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const QString &app_id,
                       const QVariantMap &options,
                       QVariantMap &results);
    QVariantMap BindShortCuts(const QDBusObjectPath &handle,
                              const QDBusObjectPath &session_handle,
                              const QVariantMap &shortcuts,
                              const QString &parent_window,
                              const QVariantMap &options);
    QVariantMap ListShortCuts(const QDBusObjectPath &handle, const QDBusObjectPath &session_handle);
};
