// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSCALLMAININTERFACE_H
#define DBUSCALLMAININTERFACE_H

#include <QObject>

#include <mutex>

class QDBusInterface;

class DbusCallMainInterface
{
public:
    inline static DbusCallMainInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DbusCallMainInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DbusCallMainInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    //!
    //! \brief jumpWidget 显示或者隐藏弹出界面
    //! \param msgCode 命令关键字
    //! \return
    //!
    void jumpWidget(QString msgCode);

protected:
    DbusCallMainInterface();

private:
    /**
     * @brief init:初始化DBus
     */
    void init();

private:
    static std::atomic<DbusCallMainInterface *> s_Instance;
    static std::mutex m_mutex;

    QDBusInterface       *mp_Iface;
};

#endif // DBUSCALLMAININTERFACE_H
