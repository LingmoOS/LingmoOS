// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTROLINTERFACE_H
#define CONTROLINTERFACE_H

#include <QObject>

#include <mutex>

class QDBusInterface;
class ControlInterface : public QObject
{
    Q_OBJECT
public:
    inline static ControlInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        ControlInterface *sin = s_instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_instance.load();

            if (!sin) {
                sin = new ControlInterface();
                s_instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief disableOutDevice 禁用设备
     * @param devInfo 设备信息
     * @return
     */
    void disableOutDevice(const QString &devInfo);
    /**
     * @brief disableOutDevice 禁用设备
     * @return
     */
    void disableInDevice();
    /**
     * @brief updateWakeup 更新唤醒
     * @param devInfo 设备信息
     * @return
     */
    void updateWakeup(const QString &devInfo);
    /**
     * @brief seMonitorWorkingFlag 在数据库里设置设备是否监控
     * @param flag 是否监控标志
     * @return
     */
    void setMonitorWorkingDBFlag(bool flag);
    /**
     * @brief monitorWorkingBDFlag 设备是否监控
     * @return 是否监控标志
     */
    bool monitorWorkingDBFlag();

signals:
    void sigFinished(bool bsuccess, QString msg);
    void sigUpdate();

protected:
    ControlInterface();

private:
    /**
     * @brief init:初始化DBus
     */
    void init();

private:
    static std::atomic<ControlInterface *> s_instance;
    static std::mutex m_mutex;

    QDBusInterface       *m_iface;
};

#endif // CONTROLINTERFACE_H
