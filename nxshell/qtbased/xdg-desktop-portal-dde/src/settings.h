// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <qdbusextratypes.h>
#include <qobjectdefs.h>

class SettingsPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Settings")
    Q_PROPERTY(uint version READ version CONSTANT)
    inline uint version() const { return 1; }

public:
    explicit SettingsPortal(QObject *parent);
    ~SettingsPortal() = default;

public slots:
    void ReadAll(const QStringList &groups);
    void Read(const QString &group, const QString &key);
    void onPaletteChanged(const QPalette &palette);

signals:
    void SettingChanged(const QString &group, const QString &key, const QDBusVariant &value);
private:
    QDBusVariant readFdoColorScheme() const;
    QDBusVariant readAccentColor() const ;
};
