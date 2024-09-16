// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETSTATUS_H
#define NETSTATUS_H

#include "private/neticonbutton.h"

#include <QBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QObject>
#include <QPointer>

class QLabel;

namespace dde {
namespace network {

class NetManager;
class NetItem;
class NetWirelessItem;

//  网络状态，仅界面无业务
class NetStatus : public QObject
{
    Q_OBJECT
public:
    explicit NetStatus(NetManager *manager, QObject *parent = nullptr);
    ~NetStatus() override;

    enum class NetworkStatus {
        Unknown = 0,
        Disabled,
        Connected,
        Disconnected,
        Connecting,
        Failed,
        ConnectNoInternet,
        WirelessDisabled,
        WiredDisabled,
        WirelessConnected,
        WiredConnected,
        WirelessDisconnected,
        WiredDisconnected,
        WirelessConnecting,
        WiredConnecting,
        WirelessConnectNoInternet,
        WiredConnectNoInternet,
        WirelessFailed,
        WiredFailed,
        WiredIpConflicted,
        WirelessIpConflicted,
        Nocable,

        Count
    };
    Q_ENUM(NetworkStatus)

    enum HoverType {
        Network,
        vpnAndProxy
    };

    struct DeviceControlItem {
        QString Id;
        bool Availabled;
        bool Enabled;
        bool Connected;
    };

    void invokeMenuItem(const QString &menuId);
    bool needShowControlCenter() const;
    const QString contextMenu(bool hasSetting) const;
    QWidget *createItemTips() const;
    QWidget *createIconWidget() const;
    QWidget *createDockItemTips();
    QWidget *createDockIconWidget();
    void refreshIcon();

    QString vpnAndProxyTips() const;
    QIcon vpnAndProxyIcon() const;

    bool hasDevice() const;
    QString networkTips() const;
    QIcon networkIcon() const;

    void setHoverTips(HoverType type);
    HoverType hoverType() const;

    void setDirection(QBoxLayout::Direction position);
    bool vpnAndProxyIconVisibel() const;

    void initQuickData();
    bool networkActive() const;
    QString quickTitle() const;
    QString quickDescription() const;
    QIcon quickIcon() const;

    inline NetworkStatus networkStatus() const { return m_networkStatus; }

public Q_SLOTS:
    void toggleNetworkActive();

protected Q_SLOTS:
    void onChildAdded(const NetItem *child);
    void onChildRemoved(const NetItem *child);
    void onStrengthLevelChanged();
    void updateStatus();
    void doUpdateStatus();

    void updateVpnAndProxyStatus();

    void updateNetworkTips();
    void updateNetworkIcon();
    void updateIconStr(const QString &iconstr);
    void nextAnimation();

    void updateQuick(unsigned wirelessStatus, unsigned wiredStatus);
    void updateQuickIconStr(const QString &iconstr);
    void nextQuickAnimation();

Q_SIGNALS:
    void networkStatusChanged(NetworkStatus networkStatus);
    void networkTipsChanged(const QString &networkTips);
    void networkIconChanged(const QIcon &icon);
    void hasDeviceChanged(bool hasDevice);

    void vpnAndProxyTipsChanged(const QString &vpnAndProxyTips);
    void vpnAndProxyIconChanged(const QIcon &icon);
    void vpnAndProxyIconVisibelChanged(const bool visible);

    void networkActiveChanged(bool active);
    void quickTitleChanged(const QString &title);
    void quickDescriptionChanged(const QString &description);
    void quickIconChanged(const QIcon &icon);

private:
    QVector<NetItem *> getDeviceConnections(unsigned type, unsigned connectType) const;
    void updateItemWidgetSize();

private:
    NetManager *m_manager;
    QString m_networkTips;
    QString m_vpnAndProxyTips;
    NetworkStatus m_networkStatus;
    bool m_hasDevice;
    unsigned m_deviceFlag;
    HoverType m_hoverType;
    DeviceControlItem m_vpnItem;
    DeviceControlItem m_proxyItem;

    QString m_networkIconStr;
    QIcon m_networkIcon;
    QIcon m_vpnAndProxyIcon;
    QString m_iphtml;

    QStringList m_animationIcon;
    int m_currentAnimation;
    QTimer *m_animationTimer;

    bool m_networkActive;
    QString m_quickTitle;
    QString m_quickDescription;
    QString m_quickIconStr;
    QIcon m_quickIcon;

    QStringList m_quickAnimationIcon;
    int m_quickCurrentAnimation;
    QTimer *m_quickAnimationTimer;
    QTimer *m_statusTimer;

    QBoxLayout *m_dockIconWidgetlayout;
    NetIconButton *m_networkBut;
    NetIconButton *m_vpnAndProxyBut;
    QLabel *m_tipsLabel;
    bool m_vpnAndProxyIconVisibel;
};

} // namespace network
} // namespace dde

#endif // NETSTATUS_H
