// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICEBASE_H
#define SERVICEBASE_H

#include <policy/policy.h>

#include <QDBusConnection>
#include <QObject>

typedef void *(*ServiceObject)(const char *path, const int len);
typedef int (*DSMRegister)(const char *name, void *data);
typedef int (*DSMUnRegister)(const char *name, void *data);

class QTimer;

class ServiceBase : public QObject
{
    Q_OBJECT
public:
    explicit ServiceBase(QObject *parent = nullptr);
    virtual ~ServiceBase();

    bool isRegister() const;
    bool isLockTimer() const;
    virtual bool registerService();
    virtual bool unregisterService();

Q_SIGNALS:
    void idleSignal();

public Q_SLOTS:
    void init(const QDBusConnection::BusType &busType, Policy *p);
    void restartTimer();

protected:
    virtual void initService();
    virtual void initThread();

public:
    Policy *policy;

protected:
    bool m_isRegister;
    bool m_isLockTimer;

    QDBusConnection::BusType m_sessionType;
    SDKType m_SDKType; // qdbus、sdbus
    QTimer *m_timer;
};

#endif // SERVICEBASE_H
