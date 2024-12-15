// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEHELPER_H
#define DEVICEHELPER_H

#include <QDBusInterface>
#include <QDir>
#include <QObject>
#include <QUrl>

// 设备管理辅助类
class DeviceHelper : public QObject
{
    Q_OBJECT

public:
    explicit DeviceHelper(QObject *parent = nullptr);
    ~DeviceHelper();

    static DeviceHelper *instance();

    // 更新所有设备信息
    void loadAllDeviceInfos();

    // 获取所有设备的挂载点
    QStringList getAllMountPoints();

    // 根据设备Id获取设备信息
    QString getMountPointByDeviceId(const QString &deviceId);

    // 根据挂载点获取设备Id
    QString getDeviceIdByMountPoint(const QString &mnp);

    // 获取所有设备Id,包括块设备和协议设备
    QStringList getAllDeviceIds();

    // 获取所有块设备Id
    QStringList getBlockDeviceIds();

    // 加载设备信息
    QVariantMap loadDeviceInfo(const QString &deviceId, bool reload = false);

    // 卸载设备
    bool detachDevice(const QString &deviceId);

    // 判断设备是否存在
    bool isExist(const QString &deviceId);

    // 判断url是否为smb网络路径
    static bool isSamba(const QUrl &url);
private:
    static DeviceHelper *m_instance;

    QStringList getProtocalDeviceIds();
private:
    QScopedPointer<QDBusInterface> m_dfmDeviceManager; // 文管设备管理DBus服务接口
    QMap<QString, QVariantMap> m_mapDevicesInfos; // 记录所有可插拔设备信息，设备id-设备信息map表
};

#endif  // DEVICEHELPER_H
