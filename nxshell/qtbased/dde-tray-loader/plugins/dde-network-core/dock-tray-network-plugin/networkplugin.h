// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKPLUGIN_H
#define NETWORKPLUGIN_H

#include "pluginsiteminterface.h"
#include "constants.h"
#include "netstatus.h"

#ifdef DOCK_API_VERSION
#if (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))
    #define USE_NEW_DOCK_API
#endif
#endif

#ifdef USE_NEW_DOCK_API
#include "pluginsiteminterface_v2.h"
#endif

#define NETWORK_KEY "network-item-key"

class QDBusMessage;

namespace dde {
namespace network {
class NetManager;
class NetView;
class NetStatus;
class QuickPanelWidget;
class DockContentWidget;

#ifdef USE_NEW_DOCK_API
class NetworkPlugin : public QObject, PluginsItemInterfaceV2
#else
class NetworkPlugin : public QObject, PluginsItemInterface
#endif
{
    Q_OBJECT

#ifdef USE_NEW_DOCK_API
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "network_v2.json")
#else
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid FILE "network.json")
#endif

public Q_SLOTS:
    void showNetworkDialog();
    void updateLockScreenStatus(bool visible);
    void updateIconColor();
    void onNotify(uint replacesId);
    void onQuickIconClicked();
    void onQuickPanelClicked();
    void onNetworkStatusChanged(NetStatus::NetworkStatus networkStatus);
    void onNetCheckAvailableChanged(const bool &netCheckAvailable);

public:
    explicit NetworkPlugin(QObject *parent = Q_NULLPTR);
    ~NetworkPlugin() Q_DECL_OVERRIDE;

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, bool checked) override;
    void refreshIcon(const QString &itemKey) override;
    void pluginStateSwitched() override;
    bool pluginIsAllowDisable() override;
    bool pluginIsDisable() override;
    const QString itemCommand(const QString &itemKey) override;
    const QString itemContextMenu(const QString &itemKey) override;
    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey) override;
    PluginSizePolicy pluginSizePolicy() const override { return Custom; }

    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, int order) override;

    void pluginSettingsChanged() override;

    void updateNetCheckVisible();

#ifdef USE_NEW_DOCK_API
    virtual Dock::PluginFlags flags() const override  { return Dock::Type_Quick | Dock::Quick_Panel_Multi | Dock::Attribute_Normal; }
    virtual QString message(const QString &msg) override;
#endif

protected:
    void positionChanged(const Dock::Position position) override;

private:
    void loadPlugin();
    void refreshPluginItemsVisible();
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPointer<QWidget> m_trayIcon;
    QPointer<QWidget> m_tipsWidget;
    dde::network::NetManager *m_manager;
    dde::network::NetView *m_netView;
    dde::network::NetStatus *m_netStatus;
    QPointer<QuickPanelWidget> m_quickPanel;
    bool m_isLockScreen;
    uint m_replacesId;
    DockContentWidget *m_dockContentWidget;
    bool m_netCheckAvailable;
    bool m_netLimited;
};
} // namespace network
} // namespace dde
#endif // NETWORKPLUGIN_H
