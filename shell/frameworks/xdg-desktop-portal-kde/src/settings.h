/*
 * SPDX-FileCopyrightText: 2018-2019 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018-2019 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_SETTINGS_H
#define XDG_DESKTOP_PORTAL_KDE_SETTINGS_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

#include <KSharedConfig>

#include "dbushelpers.h"

class DesktopPortal;
class FdoAppearanceSettings;
class KDEGlobalsSettings;
class SettingsModule;

class SettingsModule : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    ~SettingsModule() override = default;
    Q_DISABLE_COPY_MOVE(SettingsModule);
    virtual inline QString group() = 0;
    virtual VariantMapMap readAll(const QStringList &groups) = 0;
    virtual QVariant read(const QString &group, const QString &key) = 0;
    Q_SIGNAL void settingChanged(const QString &group, const QString &key, const QDBusVariant &value);
};

class SettingsPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Settings")
    Q_PROPERTY(uint version READ version CONSTANT)
public:
    explicit SettingsPortal(DesktopPortal *parent);

    uint version() const
    {
        return 1;
    }

public Q_SLOTS:
    void ReadAll(const QStringList &groups);
    void Read(const QString &group, const QString &key);

Q_SIGNALS:
    void SettingChanged(const QString &group, const QString &key, const QDBusVariant &value);

private:
    DesktopPortal *const m_parent;
    std::vector<std::unique_ptr<SettingsModule>> m_settings;
};

#endif // XDG_DESKTOP_PORTAL_KDE_SETTINGS_H
