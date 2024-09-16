// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_OBJECT_H
#define DBUS_OBJECT_H


#include <QObject>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QMutex>
#include <QReadWriteLock>

class DBusObject : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.systemMonitor")
public:
    static DBusObject &getInstance();

    /**
     * @brief registerOrNotify
     * 注册服务，成功返回true,如果失败则通知已经存在的服务并返回false
     * @return
     */
    bool registerOrNotify();

    /**
     * @brief unRegister
     * 反注册，当应用准备销毁时调用
     */
    void unRegister();


public slots:
    /**
     * @brief handleWindow
     * 接收DBus激活窗口响应接口
     */
    Q_SCRIPTABLE void handleWindow();

private:
    DBusObject(QObject *parent = nullptr);
    ~DBusObject();
    DBusObject(const DBusObject &) = delete;
    DBusObject &operator=(const DBusObject &) = delete;

private:
    QReadWriteLock internalMutex;
    static QMutex mutex;
    static QAtomicPointer<DBusObject> instance;
};

#endif // DBUS_OBJECT_H
