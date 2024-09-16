// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSALARMNOTIFY_H
#define DBUSALARMNOTIFY_H

#include <QObject>
#include <QDBusContext>
#include <QTimer>

class DBusServer : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.SystemMonitorServer")
public:
    DBusServer(QObject *parent = nullptr);
    ~DBusServer() {}
    void exitDBusServer(int msec);

public slots:
    /**
     * @brief showCpuAlarmNotify 显示CPU警告提示
     * @param allArguments 警告提示信息
     */
    void showCpuAlarmNotify(const QString &argument);

    /**
     * @brief showMemoryAlarmNotify 显示Memory警告提示
     * @param allArguments 警告提示信息
     */
    void showMemoryAlarmNotify(const QString &argument);

    /**
     * @brief showDeepinSystemMoniter 显示系统监视器主页面
     */
    void showDeepinSystemMoniter();

private:
    QTimer  m_timer;
};

#endif // DBUS_OBJECT_H
