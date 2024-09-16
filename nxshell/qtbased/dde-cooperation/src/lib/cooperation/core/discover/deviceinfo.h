// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "base/baseutils.h"

#include <QObject>
#include <QMetaType>
#include <QSharedPointer>

namespace cooperation_core {
class DeviceInfo;
}
typedef QSharedPointer<cooperation_core::DeviceInfo> DeviceInfoPointer;

namespace cooperation_core {

class DeviceInfoPrivate;
class DeviceInfo
{
public:
    enum ConnectStatus {
        Unknown,
        Connected,
        Connectable,
        Offline
    };

    enum class DiscoveryMode {
        Everyone,
        NotAllow
    };

    enum class TransMode {
        Everyone,
        OnlyConnected,
        NotAllow
    };

    enum class LinkMode {
        RightMode,
        LeftMode
    };

    explicit DeviceInfo();
    explicit DeviceInfo(const QString &ip, const QString &name);
    explicit DeviceInfo(const DeviceInfo &other);
    virtual ~DeviceInfo();

    bool isValid();

    void setOsType(deepin_cross::BaseUtils::OS_TYPE type);
    deepin_cross::BaseUtils::OS_TYPE osType() const;

    void setIpAddress(const QString &ip);
    QString ipAddress() const;

    void setConnectStatus(ConnectStatus status);
    ConnectStatus connectStatus() const;

    void setDeviceName(const QString &name);
    QString deviceName() const;

    void setTransMode(TransMode mode);
    TransMode transMode() const;

    void setDiscoveryMode(DiscoveryMode mode);
    DiscoveryMode discoveryMode() const;

    void setLinkMode(LinkMode mode);
    LinkMode linkMode() const;

    void setPeripheralShared(bool b);
    bool peripheralShared() const;

    void setClipboardShared(bool b);
    bool clipboardShared() const;

    void setCooperationEnable(bool enable);
    bool cooperationEnable() const;

    QVariantMap toVariantMap();
    static DeviceInfoPointer fromVariantMap(const QVariantMap &map);

    virtual DeviceInfo &operator=(const DeviceInfo &info);
    virtual bool operator==(const DeviceInfo &info) const;
    virtual bool operator!=(const DeviceInfo &info) const;

private:
    QSharedPointer<DeviceInfoPrivate> d { nullptr };
};

using DeviceDiscoveryMode = DeviceInfo::DiscoveryMode;
using DeviceTransMode = DeviceInfo::TransMode;
using DeviceLinkMode = DeviceInfo::LinkMode;

}   // namespace cooperation_core

Q_DECLARE_METATYPE(DeviceInfoPointer);

#endif   // DEVICEINFO_H
