// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "servicebase.h"

#include "policy/policy.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QThread>
#include <QTimer>

ServiceBase::ServiceBase(QObject *parent)
    : QObject(parent)
    , policy(nullptr)
    , m_isRegister(false)
    , m_isLockTimer(false)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    m_timer->setInterval(10 * 60 * 1000); // 设置超时时间, 默认10分钟
    connect(m_timer, &QTimer::timeout, this, &ServiceBase::idleSignal);
}

ServiceBase::~ServiceBase() { }

void ServiceBase::init(const QDBusConnection::BusType &busType, Policy *p)
{
    m_sessionType = busType;
    policy = p;
    p->setParent(this);
    // p->Print();

    m_timer->setInterval(policy->idleTime * 60 * 1000); // 设置超时时间
    connect(this, &ServiceBase::idleSignal, this, &ServiceBase::unregisterService);
    initService();
}

void ServiceBase::initService()
{
    QThread *th = new QThread();
    setParent(nullptr);
    moveToThread(th);
    connect(th, &QThread::started, this, &ServiceBase::initThread);
    th->start();
}

void ServiceBase::initThread() { }

bool ServiceBase::isRegister() const
{
    return m_isRegister;
}

bool ServiceBase::isLockTimer() const
{
    return m_isLockTimer;
}

void ServiceBase::restartTimer()
{
    m_timer->start();
}

bool ServiceBase::registerService()
{
    m_isRegister = true;
    return true;
}

bool ServiceBase::unregisterService()
{
    m_isRegister = false;
    return true;
}
