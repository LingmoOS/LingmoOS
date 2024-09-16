// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "pluginsiteminterface_v2.h"

#include <QHash>
#include <QObject>
#include <QWidget>

namespace tray {
class AbstractTrayProtocol;
class AbstractTrayProtocolHandler;
class TrayPlugin : public QObject, public PluginsItemInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterfaceV2)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "application-tray.json")

public:
    TrayPlugin(QObject *parent = nullptr);
    ~TrayPlugin();

    virtual const QString pluginName() const override;
    virtual const QString pluginDisplayName() const override;
    virtual QWidget *itemTipsWidget(const QString &itemKey) override;

    virtual void init(PluginProxyInterface *proxyInter) override;
    virtual QWidget *itemWidget(const QString &itemKey) override;
    virtual Dock::PluginFlags flags() const override;

private Q_SLOTS:
    void onTrayhandlerCreatd(QPointer<AbstractTrayProtocolHandler> handler);

private:
    QHash<QString, QWidget*> m_widget;
    QHash<QString, QWidget*> m_tooltip;
    PluginProxyInterface *m_proyInter;
};
}
