// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ONBOARDPLUGIN_H
#define ONBOARDPLUGIN_H

#include "pluginsiteminterface_v2.h"
#include "onboarditem.h"
#include "quickpanel.h"
#include "tipswidget.h"

#include <QLabel>
#include <QScopedPointer>

class OnboardPlugin : public QObject, public PluginsItemInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "onboard.json")

public:
    explicit OnboardPlugin(QObject *parent = nullptr);

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;

    void pluginStateSwitched() override;
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;

    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    const QString itemCommand(const QString &itemKey) override;
    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;
    void displayModeChanged(const Dock::DisplayMode displayMode) override;

    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;

    void pluginSettingsChanged() override;
    Dock::PluginFlags flags() const override { return Dock::Type_Quick | Dock::Quick_Panel_Single | Dock::Attribute_Normal; }

private:
    void initUI();
    void loadPlugin();
    void refreshPluginItemsVisible();

private:
    bool m_pluginLoaded;
    bool m_startupState;

    QScopedPointer<OnboardItem> m_onboardItem;
    QScopedPointer<Dock::TipsWidget> m_tipsLabel;
    QScopedPointer<QuickPanel> m_quickPanel;
};

#endif // ONBOARDPLUGIN_H
