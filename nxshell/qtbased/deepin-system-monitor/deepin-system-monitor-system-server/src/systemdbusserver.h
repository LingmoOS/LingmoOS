// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSALARMNOTIFY_H
#define DBUSALARMNOTIFY_H

#include <QObject>
#include <QDBusContext>
#include <QTimer>

class SystemDBusServer : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.SystemMonitorSystemServer")

public:
    SystemDBusServer(QObject *parent = nullptr);
    ~SystemDBusServer() override {}

    void exitDBusServer(int msec);

public Q_SLOTS:
    QString setServiceEnable(const QString &serviceName, bool enable);

private:
    QString setServiceEnableImpl(const QString &serviceName, bool enable);
    qint64 dbusCallerPid() const;
    bool checkCaller() const;

private:
    QTimer m_timer;
};

#endif  // DBUS_OBJECT_H
