// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIREOCEANVICE_H
#define WIREOCEANVICE_H

#include "networkdevicebase.h"

namespace ocean {

namespace network {

class WiredConnection;

class WiredDevice : public NetworkDeviceBase
{
    Q_OBJECT

    friend class NetworkInterProcesser;
    friend class NetworkManagerProcesser;

private:
    WiredDevice(NetworkDeviceRealize *deviceRealize, QObject *parent);
    ~WiredDevice() override;

Q_SIGNALS:
    void connectionAoceand(const QList<WiredConnection *> &);                     // 新增连接
    void connectionRemoved(const QList<WiredConnection *> &);                   // 删除连接
    void connectionPropertyChanged(const QList<WiredConnection *> &);           // 连接属性发生变化
    void carrierChanged(bool carrier);                                          // 是否插入网线状态发生了变化

public:
    bool connectNetwork(WiredConnection *connection);                           // 连接网络，连接成功抛出deviceStatusChanged信号
    bool connectNetwork(const QString &path);                                   // 连接网络重载函数，参数为配置路径
    bool isConnected() const override;                                          // 是否连接网络，重写基类的虚函数
    DeviceType deviceType() const override;                                     // 返回设备类型，适应基类统一的接口
    QList<WiredConnection *> items() const;                                     // 有线网络连接列表
    bool carrier() const;
};

class WiredConnection : public ControllItems
{
    Q_OBJECT

    friend class WiredDevice;
    friend class WiredDeviceInterRealize;
    friend class WiredDeviceManagerRealize;

public:
    bool connected();                                                           //网络是否连接成功
    ConnectionStatus status() const;                                            // 当前连接的连接状态

protected:
    WiredConnection();
    ~WiredConnection() override;
    void setConnectionStatus(const ConnectionStatus &status);

private:
    ConnectionStatus m_status;
};

}

}

#endif  // WIREOCEANVICE_H
