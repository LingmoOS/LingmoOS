// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocean-control-center/oceanuifactory.h"
#include "operation/dockpluginmodel.h"

#include "oceanuidockexport.h"
#include <QFile>
#include <QIcon>
#include <QDir>

#include <DIconTheme>

constexpr auto PLUGIN_ICON_DIR = "/usr/share/ocean-dock/icons/oceanui-setting";
constexpr auto PLUGIN_ICON_PREFIX = "oceanui-";
constexpr auto PLUGIN_ICON_DEFAULT = "oceanui_dock_plug_in";

static const QMap<QString, QString> pluginIconMap = {
    {"AiAssistant",    "oceanui_dock_assistant"}
    , {"show-desktop",   "oceanui_dock_desktop"}
    , {"onboard",        "oceanui_dock_keyboard"}
    , {"notifications",  "oceanui_dock_notify"}
    , {"shutdown",       "oceanui_dock_power"}
    , {"multitasking",   "oceanui_dock_task"}
    , {"datetime",       "oceanui_dock_time"}
    , {"system-monitor", "oceanui_dock_systemmonitor"}
    , {"grand-search",   "oceanui_dock_grandsearch"}
    , {"trash",          "oceanui_dock_trash"}
    , {"shot-start-plugin",  "oceanui_dock_shot_start_plugin"}
};

DGUI_USE_NAMESPACE;

OceanUIDockExport::OceanUIDockExport(QObject *parent) 
: QObject(parent)
, m_pluginModel(new DockPluginModel(this))
, m_dockDbusProxy(new DockDBusProxy(this))
{
    initData();
}

void OceanUIDockExport::initData()
{
    QDBusPendingReply<DockItemInfos> pluginInfos = m_dockDbusProxy->plugins();
    auto infos = pluginInfos.value();

    auto dciPaths = DIconTheme::dciThemeSearchPaths();
    dciPaths.push_back(PLUGIN_ICON_DIR);
    DIconTheme::setDciThemeSearchPaths(dciPaths);

    for (auto &info : infos) {
        QString pluginIconStr{};
        if (QFile::exists(QString(PLUGIN_ICON_DIR) + QDir::separator() + PLUGIN_ICON_PREFIX + info.name + ".dci")) {
            pluginIconStr = PLUGIN_ICON_PREFIX + info.name;
        } else if (QFile::exists(QString(PLUGIN_ICON_DIR) + QDir::separator() + info.name + ".dci")) {
            pluginIconStr =  info.name;
        } else if (QFile::exists(info.oceanui_icon)) {
            pluginIconStr = info.oceanui_icon;
        } else if (pluginIconMap.contains(info.itemKey)) {
            pluginIconStr = pluginIconMap.value(info.itemKey);
        }

        QIcon tmpIcon = QIcon::fromTheme(pluginIconStr);
        if (tmpIcon.isNull()) {
            pluginIconStr = PLUGIN_ICON_DEFAULT;
        }

        info.oceanui_icon = pluginIconStr;
    }
    m_pluginModel->resetData(infos);

    connect(m_dockDbusProxy, &DockDBusProxy::pluginVisibleChanged, m_pluginModel, &DockPluginModel::setPluginVisible);
}

DCC_FACTORY_CLASS(OceanUIDockExport)

#include "oceanuidockexport.moc"
