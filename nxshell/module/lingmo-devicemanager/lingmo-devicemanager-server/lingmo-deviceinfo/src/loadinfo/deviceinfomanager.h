// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEINFOMANAGER_H
#define DEVICEINFOMANAGER_H

#include <QObject>
#include <QMap>
#include <mutex>

class DeviceInfoManager : public QObject
{
    Q_OBJECT
public:
    inline static DeviceInfoManager *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DeviceInfoManager *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DeviceInfoManager();
                s_Instance.store(sin);
            }
        }

        return sin;

    }

    /**
     * @brief addInfo
     * @param key
     * @param value
     */
    void addInfo(const QString &key, const QString &value);

    /**
     * @brief getInfo
     * @param key
     * @return
     */
    const QString &getInfo(const QString &key);

    /**
     * @brief isInfoExisted
     * @param key
     * @return
     */
    bool isInfoExisted(const QString &key);

    /**
     * @brief isPathExisted 判断该路径下的信息是否在设备管理器里面
     * @param path
     * @return
     */
    bool isPathExisted(const QString &path);

protected:
    explicit DeviceInfoManager(QObject *parent = nullptr);

private:
    static std::atomic<DeviceInfoManager *> s_Instance;
    static std::mutex m_mutex;

    QMap<QString, QString>     m_MapInfo;
};

#endif // DEVICEINFOMANAGER_H
