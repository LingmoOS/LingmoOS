// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common.h"
#include "pluginitem.h"
#include "pluginsiteminterface_v2.h"

#include <QMenu>
#include <QLabel>
#include <QObject>
#include <QWindow>
#include <QScopedPointer>

namespace Plugin {
class EmbedPlugin;
}
namespace dock {
class WidgetPlugin : public QObject, public PluginProxyInterface
{
    Q_OBJECT

public:
    WidgetPlugin(PluginsItemInterface* pluginItem, QObject *pluginInstance);
    ~WidgetPlugin();

    // proxy interface
    void itemAdded(PluginsItemInterface * const itemInter, const QString &itemKey) override;
    void itemUpdate(PluginsItemInterface * const itemInter, const QString &itemKey) override;
    void itemRemoved(PluginsItemInterface * const itemInter, const QString &itemKey) override;
    void requestWindowAutoHide(PluginsItemInterface * const itemInter, const QString &itemKey, const bool autoHide) override;
    void requestRefreshWindowVisible(PluginsItemInterface * const itemInter, const QString &itemKey) override;
    void requestSetAppletVisible(PluginsItemInterface * const itemInter, const QString &itemKey, const bool visible) override;
    void saveValue(PluginsItemInterface * const itemInter, const QString &key, const QVariant &value) override;
    const QVariant getValue(PluginsItemInterface *const itemInter, const QString &key, const QVariant& fallback = QVariant()) override;
    void removeValue(PluginsItemInterface *const itemInter, const QStringList &keyList) override;

    static void updateDockContainerState(PluginsItemInterface *itemInter, bool onDock);
    static QString activeStateMessage(bool isActive);
public Q_SLOTS:
    void onDockPositionChanged(uint32_t position);
    void onDockDisplayModeChanged(uint32_t displayMode);
    void onDockEventMessageArrived(const QString &message);

private:
    Plugin::EmbedPlugin* getPlugin(QWidget*);
    void initConnections(Plugin::EmbedPlugin *plugin, PluginItem *pluginItem);
    int getPluginFlags();
    void pluginUpdateDockSize(const QSize &size);

    static QString messageCallback(PluginsItemInterfaceV2 *, const QString &);
    static bool supportFlag(PluginsItemInterfaceV2 *pluginV2);
    static QJsonObject getRootObj(const QString &jsonStr);
    static QString toJson(const QJsonObject &jsonObj);

private:
    PluginsItemInterface* m_pluginsItemInterface;
    QScopedPointer<PluginItem> m_pluginItem;
    QObject * m_pluginInstance;
};

}
