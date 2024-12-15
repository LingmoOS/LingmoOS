// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLIPBOARDPLUGIN_H
#define CLIPBOARDPLUGIN_H

#include <pluginsiteminterface_v2.h>
#include "quickpanelwidget.h"
#include "clipboarditem.h"

class ClipboardPlugin : public QObject, public PluginsItemInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "clipboard.json")

public:
    explicit ClipboardPlugin(QObject *parent = nullptr);
    ~ClipboardPlugin();

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
    virtual void setMessageCallback(MessageCallbackFunc cb) override { m_messageCallback = cb; }
    const QString itemCommand(const QString &itemKey) override;

private slots:
    void onClipboardVisibleChanged(bool);

private:
    QuickPanelWidget *m_quickPanelWidget;
    ClipboardItem *m_item;
    MessageCallbackFunc m_messageCallback;
};

#endif // CLIPBOARDPLUGIN_H
