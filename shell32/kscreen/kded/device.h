/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDED_DEVICE_H
#define KDED_DEVICE_H

#include <QObject>

class QDBusPendingCallWatcher;
class QDBusInterface;
class OrgFreedesktopDBusPropertiesInterface;

class Device : public QObject
{
    Q_OBJECT
public:
    static Device *self();
    static void destroy();

    bool isReady() const;
    bool isLaptop() const;
    bool isLidClosed() const;

private Q_SLOTS:
    void changed();
    void isLaptopFetched(QDBusPendingCallWatcher *watcher);
    void isLidClosedFetched(QDBusPendingCallWatcher *watcher);

Q_SIGNALS:
    void ready();
    void lidClosedChanged(bool closed);
    void resumingFromSuspend();
    void aboutToSuspend();

private:
    explicit Device(QObject *parent = nullptr);
    ~Device() override;

    void setReady();
    void fetchIsLaptop();
    void fetchLidIsClosed();

    bool m_isReady;
    bool m_isLaptop;
    bool m_isLidClosed;

    static Device *m_instance;

    OrgFreedesktopDBusPropertiesInterface *m_freedesktop;
    QDBusInterface *m_suspendSession;
};

#endif // KDED_DEVICE_H
