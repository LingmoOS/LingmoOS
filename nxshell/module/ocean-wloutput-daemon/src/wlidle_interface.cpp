// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlidle_interface.h"
#include <QtDBus/QDBusMessage>
#include <QDBusInterface>

#include <DWayland/Client/idle.h>

WlIdleInterface::WlIdleInterface(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

WlIdleInterface::~WlIdleInterface()
{
}

void WlIdleInterface::SetIdleTimeout(quint32 time)
{
    if (time == 0) {
        if (m_idleTimeout != nullptr) {
            m_idleTimeout->release();
            m_idleTimeout = nullptr;
        }
        m_timeout = 0;
        return;
    }

    if(m_timeout != time) {
        m_timeout = time;
        m_idleTimeout->deleteLater();
        m_idleTimeout = m_idle->getTimeout(m_timeout, m_seat, this);
        connect(m_idleTimeout, &IdleTimeout::idle, this, [ = ]{
            qDebug() << "idle timeout on";
            IdleTimeout(true);
        });
        connect(m_idleTimeout, &IdleTimeout::resumeFromIdle, this, [ = ]{
            qDebug() << "idle timeout off";
            IdleTimeout(false);
        });
    }
}

//void WlIdleInterface::addIdleTimeOut(Idle* idle, Seat* seat)
//{
//    auto idleTimeout = idle->getTimeout(m_idleTimeout, seat, this);
//}

void WlIdleInterface::setData( Seat* seat, Idle *idle) {
    m_seat = seat;
    m_idle = idle;
}

void WlIdleInterface::simulateUserActivity()
{
    if (m_idleTimeout)
        m_idleTimeout->simulateUserActivity();
}
