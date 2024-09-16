// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>

#include <mutex>

class QDBusInterface;

class DBusInterface
{
public:
    inline static DBusInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DBusInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DBusInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief getInfo：获取信息
     * @param key：命令关键字
     * @param info：获取的设备信息
     * @return 信息是否有效
     */
    bool getInfo(const QString &key, QString &info);

    /**
     * @brief refreshInfo 用来通知后台刷新信息
     */
    void refreshInfo();

protected:
    DBusInterface();

private:
    /**
     * @brief init:初始化DBus
     */
    void init();

private:
    static std::atomic<DBusInterface *> s_Instance;
    static std::mutex m_mutex;

    QDBusInterface       *mp_Iface;
};

#endif // DBUSINTERFACE_H
