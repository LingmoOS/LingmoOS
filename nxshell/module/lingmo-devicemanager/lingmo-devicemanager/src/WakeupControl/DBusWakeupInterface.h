// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSWAKEUPINTERFACE_H
#define DBUSWAKEUPINTERFACE_H

#include <QObject>

#include <mutex>

class QDBusInterface;

class DBusWakeupInterface
{
public:
    inline static DBusWakeupInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DBusWakeupInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DBusWakeupInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief setWakeupMachine 设置是否唤醒
     * @param unique_id 设备的唯一标识
     * @param path 设备节点路径
     * @param wakeup 可唤醒 不可唤醒
     * @return
     */
    bool setWakeupMachine(const QString &unique_id, const QString &path, bool wakeup, const QString &name);

    /**
     * @brief canWakeupMachine 获取input是否支持唤醒
     * @param path 设备节点路径
     * @return
     */
    bool canInputWakeupMachine(const QString &path);

    /**
     * @brief isWakeupMachine 获取input唤醒状态
     * @param path 设备节点路径
     * @return
     */
    bool isInputWakeupMachine(const QString &path, const QString &name);

    /**
     * @brief isNetworkWakeup 获取网卡是否支持远程唤醒
     * @param logical_name 网卡的逻辑名称
     * @return
     */
    int isNetworkWakeup(const QString &logical_name);

    /**
     * @brief setNetworkWakeup 设置网卡的唤醒功能
     * @param logical_name 网卡的逻辑名称
     * @param wake 是否唤醒
     * @return
     */
    bool setNetworkWakeup(const QString &logical_name, bool wake);

protected:
    DBusWakeupInterface();

private:
    /**
     * @brief init:初始化DBus
     */
    void init();
private:
    static std::atomic<DBusWakeupInterface *> s_Instance;
    static std::mutex                         m_mutex;
    QDBusInterface                            *mp_Iface;
    QDBusInterface                            *mp_InputIface;
};

#endif // DBUSWAKEUPINTERFACE_H
