// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WLIDLE_INTERFACE_H
#define WLIDLE_INTERFACE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusConnection>

#include <DWayland/Client/registry.h>
#include <DWayland/Client/idle.h>

using namespace KWayland::Client;

class WlIdleInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.KWayland1.Idle")

public:
    explicit WlIdleInterface(QObject *parent=nullptr);
    virtual ~WlIdleInterface();
    bool InitDBus();
//    void addIdleTimeOut(Idle* idle, Seat* seat);

public Q_SLOTS:
    void SetIdleTimeout(quint32 time);
    void setData( Seat* seat, Idle *idle);
    void simulateUserActivity ();

signals:
    void IdleTimeout(bool idle);

private:
    quint32 m_timeout{0};
    class IdleTimeout *m_idleTimeout{nullptr};
    Seat* m_seat;
    Idle *m_idle;
};

#endif // WLOUTPUT_INTERFACE_H
