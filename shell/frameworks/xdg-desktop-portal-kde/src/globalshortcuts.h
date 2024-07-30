/*
 * SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

#include "dbushelpers.h"

class GlobalShortcutsPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_DISABLE_COPY(GlobalShortcutsPortal)
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.GlobalShortcuts")
    Q_PROPERTY(uint version READ version CONSTANT)
public:
    explicit GlobalShortcutsPortal(QObject *parent);
    ~GlobalShortcutsPortal() override;

    uint version() const;

public Q_SLOTS:
    uint BindShortcuts(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const Shortcuts &shortcuts,
                       const QString &parent_window,
                       const QVariantMap &options,
                       QVariantMap &results);
    uint CreateSession(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const QString &app_id,
                       const QVariantMap &options,
                       QVariantMap &results);
    uint ListShortcuts(const QDBusObjectPath &handle, const QDBusObjectPath &session_handle, QVariantMap &results);

Q_SIGNALS:
    void Activated(const QDBusObjectPath &session_handle, const QString &shortcutId, quint64 timestamp, const QVariantMap &unused = {});
    void Deactivated(const QDBusObjectPath &session_handle, const QString &shortcutId, quint64 timestamp, const QVariantMap &unused = {});
    void ShortcutsChanged(const QDBusObjectPath &session_handle, const Shortcuts &shortcuts);
};
