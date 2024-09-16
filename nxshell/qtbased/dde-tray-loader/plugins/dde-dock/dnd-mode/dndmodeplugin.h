// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DND_MODE_PLUGIN_H
#define DND_MODE_PLUGIN_H

#include "pluginsiteminterface_v2.h"
#include "quickpanelwidget.h"
#include "dndmodeitem.h"


class DndModePlugin : public QObject, public PluginsItemInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "dnd-mode.json")

public:
    explicit DndModePlugin(QObject *parent = nullptr);
    ~DndModePlugin();

    const QString pluginName() const Q_DECL_OVERRIDE;
    const QString pluginDisplayName() const Q_DECL_OVERRIDE;
    void init(PluginProxyInterface *proxyInter) Q_DECL_OVERRIDE;
    bool pluginIsAllowDisable() override { return true; }
    QWidget *itemWidget(const QString &itemKey) Q_DECL_OVERRIDE;
    QWidget *itemPopupApplet(const QString &itemKey) Q_DECL_OVERRIDE;
    QWidget *itemTipsWidget(const QString &itemKey) Q_DECL_OVERRIDE;
    const QString itemContextMenu(const QString &itemKey) Q_DECL_OVERRIDE;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) Q_DECL_OVERRIDE;
    int itemSortKey(const QString &itemKey) Q_DECL_OVERRIDE;
    void setSortKey(const QString &itemKey, const int order) Q_DECL_OVERRIDE;
    void refreshIcon(const QString &itemKey) Q_DECL_OVERRIDE;
    Dock::PluginFlags flags() const override { return Dock::Type_Quick | Dock::Quick_Panel_Single | Dock::Attribute_Normal; }

private:
    QuickPanelWidget *m_quickPanelWidget;
    DndModeItem *m_item;
};

#endif // DND_MODE_PLUGIN_H
