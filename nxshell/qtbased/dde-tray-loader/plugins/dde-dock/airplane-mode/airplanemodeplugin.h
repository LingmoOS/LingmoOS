// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AIRPLANEMODEPLUGIN_H
#define AIRPLANEMODEPLUGIN_H

#include "pluginsiteminterface_v2.h"
#include "quickpanelwidget.h"

#include <QScopedPointer>

#include "dtkcore_global.h"

#include "com_deepin_daemon_network.h"
#include "com_deepin_daemon_bluetooth.h"

using NetworkInter = com::deepin::daemon::Network;
using BluetoothInter = com::deepin::daemon::Bluetooth;
DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

class AirplaneModeItem;
class AirplaneModePlugin : public QObject, public PluginsItemInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "airplanemode.json")

public:
    explicit AirplaneModePlugin(QObject *parent = nullptr);
    ~AirplaneModePlugin();

    const QString pluginName() const Q_DECL_OVERRIDE;
    const QString pluginDisplayName() const Q_DECL_OVERRIDE;
    void init(PluginProxyInterface *proxyInter) Q_DECL_OVERRIDE;
    bool pluginIsAllowDisable() override { return true; }
    QWidget *itemWidget(const QString &itemKey) Q_DECL_OVERRIDE;
    QWidget *itemTipsWidget(const QString &itemKey) Q_DECL_OVERRIDE;
    QWidget *itemPopupApplet(const QString &itemKey) Q_DECL_OVERRIDE;
    const QString itemContextMenu(const QString &itemKey) Q_DECL_OVERRIDE;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) Q_DECL_OVERRIDE;
    int itemSortKey(const QString &itemKey) Q_DECL_OVERRIDE;
    void setSortKey(const QString &itemKey, const int order) Q_DECL_OVERRIDE;
    void refreshIcon(const QString &itemKey) Q_DECL_OVERRIDE;
    void pluginSettingsChanged() override;
    Dock::PluginFlags flags() const override { return Dock::Type_Quick | Dock::Quick_Panel_Single | Dock::Attribute_Normal; }
    virtual void setMessageCallback(MessageCallbackFunc cb) override { m_messageCallback = cb; }
    virtual QString message(const QString &message) override;

private:
    bool supportAirplaneMode() const;
    bool getAirplaneDconfig() const;
    void notifySupportFlagChanged(bool supportFlag);

public slots:
    void refreshAirplaneEnableState();
    void onAirplaneEnableChanged(bool enable);
    void updatePluginVisible();
    void onAirplaneModePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    AirplaneModeItem *m_item;
    NetworkInter *m_networkInter;
    BluetoothInter *m_bluetoothInter;
    DTK_CORE_NAMESPACE::DConfig *m_dconfig;
    QuickPanelWidget *m_quickPanelWidget;
    MessageCallbackFunc m_messageCallback;
};

#endif // AIRPLANEMODEPLUGIN_H
