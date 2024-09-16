// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETITEM_H
#define NETITEM_H

#include <QObject>
#include <QVariant>
#include <QVector>

namespace dde {
namespace network {
Q_NAMESPACE

#define NET_WIRED 0x80     // 有线
#define NET_WIRELESS 0x40  // 无线
#define NET_DEVICE 0x01    // 设备
#define NET_CONTROL 0x00   // 控制器
#define NET_MASK_TYPE 0xc0 // 设备类型掩码

enum NetItemType {
    RootItem,                                         // 根节点,0级,虚拟项
    WirelessControlItem = NET_WIRELESS | NET_CONTROL, // 无线开关,1级,虚拟项
    WirelessDeviceItem = NET_WIRELESS | NET_DEVICE,   // 无线设备,1级
    WirelessMineItem,                                 // 我的网络,2级,虚拟项
    WirelessOtherItem,                                // 其他网络,2级,虚拟项
    WirelessItem,                                     // 无线项,3级
    WirelessHiddenItem,                               // 无线隐藏项,3级,虚拟项
    WirelessDisabledItem,                             // 无线禁用项,1级,虚拟项
    AirplaneModeTipsItem,                             // 飞行模式提示项,1级
    WiredControlItem = NET_WIRED | NET_CONTROL,       // 有线开关,1级,虚拟项
    WiredDeviceItem = NET_WIRED | NET_DEVICE,         // 有线设备,1级
    WiredItem,                                        // 有线项,2级
    WiredDisabledItem,                                // 有线禁用项,1级,虚拟项
    VPNControlItem,                                   // vpn控制项
    VPNTipsItem,                                      // vpn提示项
    SystemProxyControlItem                            // 系统代理控制项
};

enum class NetConnectionStatus {
    UnConnected = 0x01u, // 未连接
    Connecting = 0x02u,  // 正在连接
    Connected = 0x04u,   // 已连接

    AllConnectionStatus = 0x07u //  所有连接状态
};

// 位定义用于处理优先级，低位用于同级中区分
enum NetDeviceStatus {
    Unknown = 0x000Fu,           // 未知
    NoCable = 0x001Fu,           // 无网线
    Enabled = 0x003Fu,           // 启用
    Unavailable = 0x00FFu,       // 不可用 优先级同禁用
    Disabled = 0x00FFu,          // 禁用
    ObtainIpFailed = 0x01FFu,    // 获取IP失败
    ConnectFailed = 0x03FFu,     // 连接失败
    Disconnected = 0x07FFu,      // 未连接
    Connected = 0x0FFFu,         // 已连接
    ConnectNoInternet = 0x1FFFu, // 已连接但无网络
    IpConflicted = 0x3FFFu,      // IP冲突
    Connecting = 0x7FFFu,        // 连接中
    ObtainingIP = 0x7FF1u,       // 获取IP
    Authenticating = 0x7FF2u,    // 认证中
};

Q_FLAG_NS(NetDeviceStatus);

void NetItemRegisterMetaType();

// 数据项,主线程中(子线程创建,最终会移到主线程中)
class NetItem : public QObject
{
    Q_OBJECT

public:
    QString id() const;
    virtual NetItemType itemType() const;
    virtual QVariant sortValue() const;
    virtual QString name() const;
    virtual void retranslateUi();

    virtual NetItem *getChild(int childPos) const;
    virtual int getChildIndex(const NetItem *child) const;

    inline int getChildrenNumber() const { return m_children.size(); }

    inline QVector<NetItem *>::const_iterator childBegin() const { return m_children.begin(); }

    inline QVector<NetItem *>::const_iterator childEnd() const { return m_children.end(); }

    inline const QVector<NetItem *> getchildren() const { return m_children; }

    inline int getIndex() const { return (getParent() ? getParent()->getChildIndex(this) : -1); }

    inline NetItem *getParent() const { return qobject_cast<NetItem *>(parent()); }

    template<class T>
    static inline T *toItem(QObject *item)
    {
        return qobject_cast<T *>(item);
    }

protected:
    explicit NetItem(const QString &id);
    ~NetItem() override;
    virtual bool addChild(NetItem *child, int index = -1);
    virtual void removeChild(NetItem *child);
    virtual void removeChild(int index);
    void updateName(const QString &name);

Q_SIGNALS:
    void nameChanged(const QString &name);
    void dataChanged();
    void childAboutToBeAdded(const NetItem *parent, int pos);
    void childAdded(const NetItem *child);
    void childAboutToBeRemoved(const NetItem *parent, int pos);
    void childRemoved(const NetItem *child);

protected Q_SLOTS:
    void emitDataChanged();

private:
    QString m_name;
    QVector<NetItem *> m_children;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetControlItem : public NetItem
{
    Q_OBJECT

public:
    bool isEnabled() const;
    bool enabledable() const;

Q_SIGNALS:
    void enabledChanged(const bool);
    void enabledableChanged(const bool);

protected:
    void updateEnabled(const bool enabled);
    void updateEnabledable(const bool enabledable);
    explicit NetControlItem(const QString &id);
    ~NetControlItem() Q_DECL_OVERRIDE;

private:
    bool m_enabled;
    bool m_enabledable;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetDeviceItem : public NetControlItem
{
    Q_OBJECT

public:
    NetDeviceStatus status() const;
    QStringList ips() const;

Q_SIGNALS:
    void statusChanged(const NetDeviceStatus status);
    void ipsChanged(const QStringList &ips);

protected:
    void updateStatus(const NetDeviceStatus status);
    void updateIps(const QStringList &ips);
    explicit NetDeviceItem(const QString &id);
    ~NetDeviceItem() Q_DECL_OVERRIDE;

private:
    NetDeviceStatus m_status;
    QStringList m_ips;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWiredControlItem : public NetControlItem
{
    Q_OBJECT

public:
    NetItemType itemType() const override;
    void retranslateUi() override;

protected:
    explicit NetWiredControlItem(const QString &id);

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWirelessControlItem : public NetControlItem
{
    Q_OBJECT

public:
    NetItemType itemType() const override;
    void retranslateUi() override;

protected:
    explicit NetWirelessControlItem(const QString &id);

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetTipsItem : public NetItem
{
    Q_OBJECT
public:
    NetItemType itemType() const override;
    const QString &linkActivatedText() const { return m_linkActivatedText; }
    const bool tipsLinkEnabled() const { return m_tipsLinkEnabled; }

protected:
    explicit NetTipsItem(const QString &id, const QString &linkActivatedText, const bool tipsLinkEnabled);

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;

private:
    QString m_linkActivatedText;
    bool m_tipsLinkEnabled;
};

class NetVPNControlItem : public NetDeviceItem
{
    Q_OBJECT

public:
    NetItemType itemType() const override;
    void retranslateUi() override;
    bool isExpanded() const;

Q_SIGNALS:
    void expandedChanged(bool isExpanded);

protected:
    explicit NetVPNControlItem(const QString &id);
    void updateExpanded(bool isExpanded);

private:
    bool m_isExpanded;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetVPNTipsItem : public NetTipsItem
{
    Q_OBJECT
public:
    NetItemType itemType() const override;
    void retranslateUi() override;

protected:
    explicit NetVPNTipsItem(const QString &id, const QString &linkActivatedText, const bool tipsLinkEnabled);

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetSystemProxyControlItem : public NetControlItem
{
    Q_OBJECT

public:
    NetItemType itemType() const override;
    void retranslateUi() override;

protected:
    explicit NetSystemProxyControlItem(const QString &id);

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWiredDeviceItem : public NetDeviceItem
{
    Q_OBJECT

public:
    NetItemType itemType() const override;

protected:
    using NetDeviceItem::NetDeviceItem;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWirelessDeviceItem : public NetDeviceItem
{
    Q_OBJECT
public:
    NetItemType itemType() const override;
    bool isApMode() const;

Q_SIGNALS:
    void apModeChanged(bool apMode);

protected:
    void updateApMode(bool apMode);
    explicit NetWirelessDeviceItem(const QString &id);

private:
    bool m_isApMode;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWirelessOtherItem : public NetItem
{
    Q_OBJECT
public:
    NetItemType itemType() const override;
    void retranslateUi() override;
    bool isExpanded() const;

Q_SIGNALS:
    void expandedChanged(bool isExpanded);

protected:
    void updateExpanded(bool isExpanded);

    explicit NetWirelessOtherItem(const QString &id);

private:
    bool m_isExpanded;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWirelessMineItem : public NetItem
{
    Q_OBJECT
public:
    NetItemType itemType() const override;
    void retranslateUi() override;

protected:
    explicit NetWirelessMineItem(const QString &id);

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWirelessDisabledItem : public NetItem
{
    Q_OBJECT
public:
    NetItemType itemType() const override;
    void retranslateUi() override;

protected:
    using NetItem::NetItem;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetAirplaneModeTipsItem : public NetTipsItem
{
    Q_OBJECT
public:
    NetItemType itemType() const override;
    void retranslateUi() override;

protected:
    explicit NetAirplaneModeTipsItem(const QString &id, const QString &linkActivatedText, const bool tipsLinkEnabled);

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};


class NetWiredDisabledItem : public NetItem
{
    Q_OBJECT
public:
    NetItemType itemType() const override;
    void retranslateUi() override;

protected:
    using NetItem::NetItem;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWiredItem : public NetItem
{
    Q_OBJECT

public:
    NetItemType itemType() const override;
    NetConnectionStatus status() const;

Q_SIGNALS:
    void statusChanged(NetConnectionStatus status);

protected:
    void updateStatus(NetConnectionStatus status);
    explicit NetWiredItem(const QString &id);
    ~NetWiredItem() Q_DECL_OVERRIDE;

private:
    NetConnectionStatus m_status;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWirelessHiddenItem : public NetItem
{
    Q_OBJECT

public:
    NetItemType itemType() const override;
    QVariant sortValue() const override;
    void retranslateUi() override;

protected:
    explicit NetWirelessHiddenItem(const QString &id);
    ~NetWirelessHiddenItem() Q_DECL_OVERRIDE;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

class NetWirelessItem : public NetItem
{
    Q_OBJECT

    Q_PROPERTY(int strength READ strength NOTIFY strengthChanged)
    Q_PROPERTY(bool secure READ isSecure NOTIFY secureChanged)

public:
    NetItemType itemType() const override;
    QVariant sortValue() const override;

    uint flags() const;
    int strength() const;
    int strengthLevel() const;
    bool isSecure() const;
    NetConnectionStatus status() const;
    bool hasConnection() const;

Q_SIGNALS:
    void flagsChanged(uint flags);
    void strengthChanged(int strength);
    void strengthLevelChanged(int strengthLevel);
    void secureChanged(bool secure);
    void statusChanged(NetConnectionStatus status);
    void hasConnectionChanged(bool hasConnection);

protected:
    void updateFlags(uint flags);
    void updateStrength(int strength);
    void updateSecure(bool secure);
    void updateStatus(NetConnectionStatus status);
    void updateHasConnection(bool hasConnection);

    explicit NetWirelessItem(const QString &id);
    ~NetWirelessItem() Q_DECL_OVERRIDE;

private:
    uint m_flags;
    int m_strength;
    int m_strengthLevel;
    bool m_secure;
    NetConnectionStatus m_status;
    bool m_hasConnection;

    friend class NetManagerPrivate;
    friend class NetManagerThreadPrivate;
};

} // namespace network
} // namespace dde

Q_DECLARE_METATYPE(dde::network::NetItemType)
Q_DECLARE_METATYPE(dde::network::NetConnectionStatus)
Q_DECLARE_METATYPE(dde::network::NetDeviceStatus)

#endif //  NETWORKAPPLETMODEL_H
