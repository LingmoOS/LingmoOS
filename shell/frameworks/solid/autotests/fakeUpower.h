/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_FAKE_UPOWER_H
#define SOLID_FAKE_UPOWER_H

#include <QDBusObjectPath>
#include <QList>
#include <QString>

class FakeUpower : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool IsDocked READ isDocked)
    Q_PROPERTY(bool OnBattery READ onBattery)
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.UPower")
public:
    explicit FakeUpower(QObject *parent);

    QString daemonVersion() const;
    bool isDocked() const;
    bool lidIsClosed() const;
    bool lidIsPresent() const;
    bool onBattery() const;
    void setOnBattery(bool onBattery);

    bool m_onBattery;

    void emitPropertiesChanged(const QString &name, const QVariant &value);
public Q_SLOTS:
    QList<QDBusObjectPath> EnumerateDevices();
    QString GetCriticalAction();
    QDBusObjectPath GetDisplayDevice();
};

#endif // SOLID_FAKE_UPOWER_H
