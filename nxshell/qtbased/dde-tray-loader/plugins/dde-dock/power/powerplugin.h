// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POWERPLUGIN_H
#define POWERPLUGIN_H

#include "dbus/dbuspower.h"
#include "pluginsiteminterface_v2.h"
#include "powerstatuswidget.h"
#include "tipswidget.h"

#include <com_deepin_system_systempower.h>

#include <QLabel>

using SystemPowerInter = com::deepin::system::Power;

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

class PowerPlugin : public QObject, PluginsItemInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "power.json")

public:
    explicit PowerPlugin(QObject *parent = nullptr);

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;
    void pluginStateSwitched() override;
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;
    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;
    void refreshIcon(const QString &itemKey) override;
    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;
    void pluginSettingsChanged() override;
    virtual void setMessageCallback(MessageCallbackFunc cb) override { m_messageCallback = cb; }
    virtual QString message(const QString &message) override;

private:
    void updateBatteryVisible();
    void loadPlugin();
    void refreshPluginItemsVisible();
    void onDConfigValueChanged(const QString &key);
    void refreshTipsData();
    void batteryTimeToDisplayData(const qulonglong &time, uint &hour, uint &minute);
    void notifySupportFlagChanged(bool supportFlag);

private:
    bool m_pluginLoaded;
    bool m_showTimeToFull;

    QScopedPointer<PowerStatusWidget> m_powerStatusWidget;
    QScopedPointer<Dock::TipsWidget> m_tipsLabel;

    SystemPowerInter *m_systemPowerInter;
    DBusPower *m_powerInter;
    QTimer *m_batteryStateChangedTimer;
    MessageCallbackFunc m_messageCallback;

    Dtk::Core::DConfig *m_config = nullptr;
};

#endif // POWERPLUGIN_H
