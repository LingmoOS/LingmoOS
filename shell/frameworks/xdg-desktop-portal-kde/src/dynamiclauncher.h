// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class DynamicLauncherPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.DynamicLauncher")

    Q_PROPERTY(uint version MEMBER m_version CONSTANT)
    const uint m_version = 1;

    Q_PROPERTY(uint SupportedLauncherTypes MEMBER m_supportedTypes CONSTANT)
    const uint m_supportedTypes = uint(Type::Application) | uint(Type::Webapp);

public:
    enum class Type { Application = 1, Webapp = 2 };

    explicit DynamicLauncherPortal(QObject *parent = nullptr);

public Q_SLOTS:
    uint PrepareInstall(const QDBusObjectPath &handle,
                        const QString &app_id,
                        const QString &parent_window,
                        const QString &name,
                        const QDBusVariant &icon_v,
                        const QVariantMap &options,
                        QVariantMap &results);
    uint RequestInstallToken(const QString &app_id, const QVariantMap &options);
};
