// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETITEM_H
#define NETITEM_H

#include "nettype.h"

#include <QList>
#include <QObject>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
QT_END_NAMESPACE

namespace ocean {
namespace network {
class NetItemPrivate;

// 数据项,主线程中(子线程创建,最终会移到主线程中)
class NetItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(NetType::NetItemType itemType READ itemType)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QVector<NetItem *> children READ getChildren NOTIFY childrenChanged DESIGNABLE false)

public:
    QString id() const;
    virtual NetType::NetItemType itemType() const;
    virtual QString name() const;

public Q_SLOTS:
    virtual NetItem *getChild(int childPos) const;
    virtual int getChildIndex(const NetItem *child) const;
    int getChildrenNumber() const;
    const QVector<NetItem *> &getChildren() const;
    int getIndex() const;
    NetItem *getParent() const;

public:
    template<class T>
    static inline T *toItem(QObject *item)
    {
        return qobject_cast<T *>(item);
    }

    template<class T>
    static inline const T *toItem(const QObject *item)
    {
        return qobject_cast<const T *>(item);
    }

Q_SIGNALS:
    void idChanged(const QString &newID, const QString &oldID);
    void nameChanged(const QString &name);
    void dataChanged();
    void childAboutToBeAoceand(const NetItem *parent, int pos);
    void childAoceand(const NetItem *child);
    void childAboutToBeRemoved(const NetItem *parent, int pos);
    void childRemoved(const NetItem *child);
    void childrenChanged();

protected:
    explicit NetItem(NetItemPrivate *p, const QString &id);
    ~NetItem() override;

protected:
    NetItemPrivate *dptr;
    friend class NetItemPrivate;
};

class NetControlItem : public NetItem
{
    Q_OBJECT
    Q_PROPERTY(bool isEnabled READ isEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool enabledable READ enabledable NOTIFY enabledableChanged)
public:
    bool isEnabled() const;
    bool enabledable() const;

Q_SIGNALS:
    void enabledChanged(const bool);
    void enabledableChanged(const bool);

protected:
    using NetItem::NetItem;
};

class NetDeviceItem : public NetControlItem
{
    Q_OBJECT
    Q_PROPERTY(NetType::NetDeviceStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QStringList ips READ ips NOTIFY ipsChanged)
    Q_PROPERTY(int pathIndex READ pathIndex NOTIFY pathIndexChanged)
public:
    NetType::NetDeviceStatus status() const;
    QStringList ips() const;
    int pathIndex() const;

Q_SIGNALS:
    void statusChanged(const NetType::NetDeviceStatus status);
    void ipsChanged(const QStringList &ips);
    void pathIndexChanged(const int &pathIndex);

protected:
    using NetControlItem::NetControlItem;
};

class NetWiredControlItem : public NetControlItem
{
    Q_OBJECT

public:
    QString name() const override;

protected:
    using NetControlItem::NetControlItem;
};

class NetWirelessControlItem : public NetControlItem
{
    Q_OBJECT

public:
    QString name() const override;

protected:
    using NetControlItem::NetControlItem;
};

class NetTipsItem : public NetItem
{
    Q_OBJECT
public:
    NetType::NetItemType itemType() const override { return NetType::NetItemType::Item; }

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

class NetWiredDeviceItem : public NetDeviceItem
{
    Q_OBJECT

protected:
    using NetDeviceItem::NetDeviceItem;
};

class NetWirelessDeviceItem : public NetDeviceItem
{
    Q_OBJECT
    Q_PROPERTY(bool apMode READ apMode NOTIFY apModeChanged)
public:
    bool apMode() const;

Q_SIGNALS:
    void apModeChanged(bool apMode) const;

protected:
    using NetDeviceItem::NetDeviceItem;
};

// class NetAirplaneModeTipsItem : public NetTipsItem
// {
//     Q_OBJECT
// public:
//     NetType::NetItemType itemType() const override;

// protected:
//     explicit NetAirplaneModeTipsItem(const QString &id, const QString &linkActivatedText, const bool tipsLinkEnabled);

//     friend class NetManagerPrivate;
//     friend class NetManagerThreadPrivate;
// };

///////////////////////////////////////////////
class NetConnectionItem : public NetItem
{
    Q_OBJECT
    Q_PROPERTY(NetType::NetConnectionStatus status READ status NOTIFY statusChanged)
public:
    NetType::NetConnectionStatus status() const;

Q_SIGNALS:
    void statusChanged(NetType::NetConnectionStatus status);

protected:
    using NetItem::NetItem;
};

class NetWiredItem : public NetConnectionItem
{
    Q_OBJECT
protected:
    using NetConnectionItem::NetConnectionItem;
};

class NetWiredDisabledItem : public NetItem
{
    Q_OBJECT
protected:
    using NetItem::NetItem;
};

class NetWirelessHiddenItem : public NetItem
{
    Q_OBJECT
public:
    QString name() const override;

protected:
    using NetItem::NetItem;
};

class NetWirelessItem : public NetConnectionItem
{
    Q_OBJECT

    Q_PROPERTY(uint flags READ flags NOTIFY flagsChanged)
    Q_PROPERTY(int strength READ strength NOTIFY strengthChanged)
    Q_PROPERTY(int strengthLevel READ strengthLevel NOTIFY strengthLevelChanged)
    Q_PROPERTY(bool secure READ isSecure NOTIFY secureChanged)
    Q_PROPERTY(bool hasConnection READ hasConnection NOTIFY hasConnectionChanged)

public:
    uint flags() const;
    int strength() const;
    int strengthLevel() const;
    bool isSecure() const;
    bool hasConnection() const;

Q_SIGNALS:
    void flagsChanged(uint flags);
    void strengthChanged(int strength);
    void strengthLevelChanged(int strengthLevel);
    void secureChanged(bool secure);
    void hasConnectionChanged(bool hasConnection);

protected:
    using NetConnectionItem::NetConnectionItem;
};

class NetWirelessOtherItem : public NetItem
{
    Q_OBJECT
public:
    bool isExpanded() const;

Q_SIGNALS:
    void expandedChanged(bool expanded);

protected:
    using NetItem::NetItem;
};

class NetWirelessMineItem : public NetItem
{
    Q_OBJECT
protected:
    using NetItem::NetItem;
};

class NetWirelessDisabledItem : public NetItem
{
    Q_OBJECT
protected:
    using NetItem::NetItem;
};

///////////////////////////////////////////

class NetVPNControlItem : public NetControlItem
{
    Q_OBJECT

public:
    bool isExpanded() const;

Q_SIGNALS:
    void expandedChanged(bool expanded);

protected:
    using NetControlItem::NetControlItem;
};

class NetVPNTipsItem : public NetTipsItem
{
    Q_OBJECT
public:
    QString name() const override;

protected:
    using NetTipsItem::NetTipsItem;
};

class NetSystemProxyControlItem : public NetControlItem
{
    Q_OBJECT
    Q_PROPERTY(NetType::ProxyMethod method READ method NOTIFY methodChanged)
    Q_PROPERTY(QString autoProxy READ autoProxy NOTIFY autoProxyChanged)
    Q_PROPERTY(QVariantMap manualProxy READ manualProxy NOTIFY manualProxyChanged)
public:
    NetType::ProxyMethod method() const;
    const QString &autoProxy() const;
    const QVariantMap &manualProxy() const;

Q_SIGNALS:
    void methodChanged(NetType::ProxyMethod method);
    void autoProxyChanged(const QString &autoProxy);
    void manualProxyChanged(const QVariantMap &manualProxy);

protected:
    using NetControlItem::NetControlItem;
};

class NetAppProxyControlItem : public NetControlItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap config READ config NOTIFY configChanged)
public:
    const QVariantMap &config() const;

Q_SIGNALS:
    void configChanged(const QVariantMap &config);

protected:
    using NetControlItem::NetControlItem;
};

class NetHotspotControlItem : public NetControlItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap config READ config NOTIFY configChanged)
    Q_PROPERTY(QStringList optionalDevice READ optionalDevice NOTIFY optionalDeviceChanged)
    Q_PROPERTY(QStringList shareDevice READ shareDevice NOTIFY shareDeviceChanged)
public:
    const QVariantMap &config() const;
    const QStringList &optionalDevice() const;
    const QStringList &shareDevice() const;

Q_SIGNALS:
    void configChanged(const QVariantMap &config);
    void optionalDeviceChanged(const QStringList &optionalDevice);
    void shareDeviceChanged(const QStringList &shareDevice);

protected:
    using NetControlItem::NetControlItem;
};

class NetAirplaneControlItem : public NetControlItem
{
    Q_OBJECT
protected:
    using NetControlItem::NetControlItem;
};

// DSL
class NetDSLControlItem : public NetControlItem
{
    Q_OBJECT
protected:
    using NetControlItem::NetControlItem;
};

// DetailsItem
class NetDetailsItem : public NetItem
{
    Q_OBJECT
protected:
    using NetItem::NetItem;
};

class NetDetailsInfoItem : public NetItem
{
    Q_OBJECT
    Q_PROPERTY(QList<QStringList> details READ details NOTIFY detailsChanged)
    Q_PROPERTY(int index READ index NOTIFY indexChanged)
public:
    const QList<QStringList> &details() const;
    const int &index() const;

Q_SIGNALS:
    void detailsChanged(const QList<QStringList> &details);
    void indexChanged(const int &index);

protected:
    using NetItem::NetItem;
};
} // namespace network
} // namespace ocean

#endif //  NETWORKAPPLETMODEL_H
