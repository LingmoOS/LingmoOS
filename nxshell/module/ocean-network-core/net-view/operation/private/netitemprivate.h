// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETITEMPRIVATE_H
#define NETITEMPRIVATE_H

#include "netitem.h"

#include <QList>

namespace ocean {
namespace network {
#define NetItemNew(TYPE, ID) static_cast<Net##TYPE##Private *>(NetItemPrivate::New(NetType::NetItemType::TYPE, ID))

// 数据项,主线程中(子线程创建,最终会移到主线程中)
class NetItemPrivate
{
public:
    static NetItemPrivate *New(NetType::NetItemType type, const QString &id);

    NetItem *item();
    QString id() const;
    virtual NetType::NetItemType itemType() const;
    virtual QString name() const;

    virtual NetItem *getChild(int childPos) const;
    virtual int getChildIndex(const NetItem *child) const;

    inline int getChildrenNumber() const { return m_children.size(); }

    inline const QVector<NetItem *> &getChildren() const { return m_children; }

    inline int getIndex() const { return m_parent ? m_parent->getChildren().indexOf(m_item) : -1; }

    NetItem *getParent() const { return m_parent; }

    NetItemPrivate *getParentPrivate() const { return m_parent ? m_parent->dptr : nullptr; }

    template<class T>
    static inline T *toItem(NetItemPrivate *item)
    {
        return dynamic_cast<T *>(item);
    }

    template<class T>
    static inline T *toItem(NetItem *item)
    {
        return dynamic_cast<T *>(item->dptr);
    }

public:
    virtual ~NetItemPrivate();
    virtual bool addChild(NetItemPrivate *child, int index = -1);
    void removeChild(NetItemPrivate *child);
    void updatename(const QString &name);
    void updateid(const QString &id);

protected:
    void emitDataChanged();
    explicit NetItemPrivate();

protected:
    NetItem *m_item;
    NetItem *m_parent;
    QVector<NetItem *> m_children;
    QString m_name;
    friend class NetManager;
    friend class NetManagerPrivate;
};

class NetControlItemPrivate : public NetItemPrivate
{
public:
    bool isEnabled() const;
    bool enabledable() const;
    void updateenabled(const bool enabled);
    void updateenabledable(const bool enabledable);

protected:
    explicit NetControlItemPrivate();

private:
    bool m_enabled;
    bool m_enabledable;
};

class NetDeviceItemPrivate : public NetControlItemPrivate
{
public:
    NetType::NetDeviceStatus status() const;
    QStringList ips() const;
    int pathIndex() const;
    // QString path() const;
    void updatestatus(const NetType::NetDeviceStatus status);
    void updateips(const QStringList &ips);
    void updatepathIndex(const int &pathIndex);

protected:
    explicit NetDeviceItemPrivate();

private:
    NetType::NetDeviceStatus m_status;
    QStringList m_ips;
    int m_pathIndex;
};

class NetWiredControlItemPrivate : public NetControlItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetControlItemPrivate::NetControlItemPrivate;
};

class NetWirelessControlItemPrivate : public NetControlItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetControlItemPrivate::NetControlItemPrivate;
};

class NetWiredDeviceItemPrivate : public NetDeviceItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetDeviceItemPrivate::NetDeviceItemPrivate;
};

class NetWirelessDeviceItemPrivate : public NetDeviceItemPrivate
{

public:
    NetType::NetItemType itemType() const override;
    bool apMode() const;
    void updateapMode(bool apMode);

protected:
    explicit NetWirelessDeviceItemPrivate();

private:
    bool m_apMode;

    friend class NetItemPrivate;
};

class NetConnectionItemPrivate : public NetItemPrivate
{
public:
    NetType::NetItemType itemType() const override { return NetType::NetItemType::ConnectionItem; }

    NetType::NetConnectionStatus status() const;
    void updatestatus(NetType::NetConnectionStatus status);

protected:
    explicit NetConnectionItemPrivate();

private:
    NetType::NetConnectionStatus m_status;
    friend class NetItemPrivate;
};

class NetWiredItemPrivate : public NetConnectionItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetConnectionItemPrivate::NetConnectionItemPrivate;
};

class NetWirelessMineItemPrivate : public NetItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetItemPrivate::NetItemPrivate;
};

class NetWirelessOtherItemPrivate : public NetItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

    bool isExpanded() const;
    void updateexpanded(bool expanded);

protected:
    explicit NetWirelessOtherItemPrivate();

private:
    bool m_expanded;
    friend class NetItemPrivate;
};

class NetWirelessHiddenItemPrivate : public NetItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetItemPrivate::NetItemPrivate;
};

class NetWirelessItemPrivate : public NetConnectionItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

    uint flags() const;
    int strength() const;
    int strengthLevel() const;
    bool isSecure() const;
    bool hasConnection() const;
    void updateflags(uint flags);
    void updatestrength(int strength);
    void updatesecure(bool secure);
    void updatehasConnection(bool hasConnection);

protected:
    explicit NetWirelessItemPrivate();

private:
    uint m_flags;
    int m_strength;
    int m_strengthLevel;
    bool m_secure;
    bool m_hasConnection;

    friend class NetItemPrivate;
};

// VPN
class NetVPNControlItemPrivate : public NetDeviceItemPrivate
{
public:
    NetType::NetItemType itemType() const override;
    bool isExpanded() const;
    void updateexpanded(bool expanded);

protected:
    explicit NetVPNControlItemPrivate();

private:
    bool m_expanded;

    friend class NetItemPrivate;
};

// 系统代理

class NetSystemProxyControlItemPrivate : public NetControlItemPrivate
{
public:
    NetType::NetItemType itemType() const override;
    NetType::ProxyMethod method() const;
    const QString &autoProxy() const;
    const QVariantMap &manualProxy() const;
    void updatemethod(NetType::ProxyMethod method);
    void updateautoProxy(const QString &autoProxy);
    void updatemanualProxy(const QVariantMap &manualProxy);

protected:
    using NetControlItemPrivate::NetControlItemPrivate;

private:
    NetType::ProxyMethod m_method;
    QString m_autoProxy;
    QVariantMap m_manualProxy;
};

// 应用代理
class NetAppProxyControlItemPrivate : public NetControlItemPrivate
{
public:
    NetType::NetItemType itemType() const override;
    const QVariantMap &config() const;
    void updateconfig(const QVariantMap &config);

protected:
    using NetControlItemPrivate::NetControlItemPrivate;

private:
    QVariantMap m_config;
};

// DSL
class NetDSLControlItemPrivate : public NetControlItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetControlItemPrivate::NetControlItemPrivate;

    friend class NetItemPrivate;
};

// Hotspot
class NetHotspotControlItemPrivate : public NetControlItemPrivate
{
public:
    NetType::NetItemType itemType() const override;
    const QVariantMap &config() const;
    const QStringList &shareDevice() const;
    const QStringList &optionalDevice() const;
    void updateconfig(const QVariantMap &config);
    void updateshareDevice(const QStringList &shareDevice);
    void updateoptionalDevice(const QStringList &optionalDevice);

protected:
    using NetControlItemPrivate::NetControlItemPrivate;

private:
    QVariantMap m_config;
    QStringList m_shareDevice;
    QStringList m_optionalDevice;
};

// Airplane
class NetAirplaneControlItemPrivate : public NetControlItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetControlItemPrivate::NetControlItemPrivate;

    friend class NetItemPrivate;
};

// DetailsItem
class NetDetailsItemPrivate : public NetItemPrivate
{
public:
    NetType::NetItemType itemType() const override;

protected:
    using NetItemPrivate::NetItemPrivate;
};

class NetDetailsInfoItemPrivate : public NetItemPrivate
{
public:
    NetType::NetItemType itemType() const override;
    const QList<QStringList> &details() const;
    const int &index() const;
    void updatedetails(const QList<QStringList> &details);
    void updateindex(const int &index);

protected:
    using NetItemPrivate::NetItemPrivate;

protected:
    QList<QStringList> m_details;
    int m_index;
    friend class NetItemPrivate;
};
} // namespace network
} // namespace ocean

#endif //  NETITEMPRIVATE_H
