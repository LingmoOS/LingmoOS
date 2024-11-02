/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "remote-config.h"
#include <config-loader.h>
#include <types.h>
#include <island.h>

RemoteConfig::RemoteConfig(QObject *parent) : RemoteConfigSource(parent)
        , m_mergeIcons(GeneralConfigDefine::MergeIcons::Never)
        , m_panelLocation(GeneralConfigDefine::PanelLocation::Bottom)
        , m_panelSizePolicy(GeneralConfigDefine::PanelSizePolicy::Small)
        , m_panelAutoHide(false)
        , m_panelLock(false)
        , m_taskBarIconsShowedOn(GeneralConfigDefine::TaskBarIconsShowedOn::PanelWhereWindowIsOpen)
        , m_showPanelOnAllScreens(true)
        , m_showSystemTrayOnAllPanels(true)
{
    qRegisterMetaType<GeneralConfigDefine::MergeIcons>("GeneralConfigDefine::MergeIcons");
    qRegisterMetaType<GeneralConfigDefine::PanelLocation>("GeneralConfigDefine::PanelLocation");
    qRegisterMetaType<GeneralConfigDefine::PanelSizePolicy>("GeneralConfigDefine::PanelSizePolicy");
    qRegisterMetaType<GeneralConfigDefine::TaskBarIconsShowedOn>("GeneralConfigDefine::TaskBarIconsShowedOn");

    m_taskManagerConfig = LingmoUIQuick::ConfigLoader::getConfig("org.lingmo.panel.taskManager");
    m_systemTrayConfig = LingmoUIQuick::ConfigLoader::getConfig("org.lingmo.systemTray");
    m_mergeIcons = m_taskManagerConfig->getValue(QStringLiteral("mergeIcons")).value<GeneralConfigDefine::MergeIcons>();
    m_trayIconsInhibited = m_systemTrayConfig->getValue(QStringLiteral("trayIconsInhibited")).toStringList();

    m_taskBarIconsShowedOn = m_taskManagerConfig->getValue(QStringLiteral("taskBarIconsShowedOn")).value<GeneralConfigDefine::TaskBarIconsShowedOn>();

    connect(m_taskManagerConfig, &LingmoUIQuick::Config::configChanged, this, &RemoteConfig::onTaskManagerConfigChanged);
    connect(m_systemTrayConfig, &LingmoUIQuick::Config::configChanged, this, &RemoteConfig::onSystemTrayConfigChanged);

    m_panelConfig = LingmoUIQuick::ConfigLoader::getConfig("panel", LingmoUIQuick::ConfigLoader::Local, "lingmo-panel");
    m_panelConfig->addGroupInfo(LingmoUIQuick::Island::viewGroupName(), LingmoUIQuick::Island::viewGroupKey());
    for(auto config : m_panelConfig->children(LingmoUIQuick::Island::viewGroupName())) {
        if(config->getValue("id").toString() == "org.lingmo.panel") {
            m_mainViewConfig = config;
            break;
        }
    }
    m_showPanelOnAllScreens = m_panelConfig->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
    connect(m_panelConfig, &LingmoUIQuick::Config::configChanged, this, &RemoteConfig::onPanelConfigChanged);

    if (m_mainViewConfig) {
        updatePanelLocation();
        m_disabledWidgets = m_mainViewConfig->getValue(QStringLiteral("disabledWidgets")).toStringList();
        m_panelSizePolicy = m_mainViewConfig->getValue(QStringLiteral("panelSizePolicy")).value<GeneralConfigDefine::PanelSizePolicy>();
        m_panelAutoHide = m_mainViewConfig->getValue(QStringLiteral("panelAutoHide")).toBool();
        m_panelLock = m_mainViewConfig->getValue(QStringLiteral("lockPanel")).toBool();
        m_panelWidgets = m_mainViewConfig->getValue(QStringLiteral("widgets")).toStringList();
        m_showSystemTrayOnAllPanels = m_mainViewConfig->getValue(QStringLiteral("showSystemTrayOnAllPanels")).toBool();
        connect(m_mainViewConfig, &LingmoUIQuick::Config::configChanged, this, &RemoteConfig::onPanelViewConfigChanged);
    }
}

void RemoteConfig::setMergeIcons(GeneralConfigDefine::MergeIcons mergeIcons)
{
    if (mergeIcons != m_mergeIcons) {
        m_mergeIcons = mergeIcons;
        m_taskManagerConfig->setValue(QStringLiteral("mergeIcons"), mergeIcons);
        Q_EMIT mergeIconsChanged(m_mergeIcons);
    }
}

void RemoteConfig::setPanelLocation(GeneralConfigDefine::PanelLocation panelLocation)
{
    if (panelLocation != m_panelLocation) {
        m_panelLocation = panelLocation;

        LingmoUIQuick::Types::Pos pos;
        switch (m_panelLocation) {
            default:
            case GeneralConfigDefine::Bottom:
                pos = LingmoUIQuick::Types::Bottom;
                break;
            case GeneralConfigDefine::Left:
                pos = LingmoUIQuick::Types::Left;
                break;
            case GeneralConfigDefine::Top:
                pos = LingmoUIQuick::Types::Top;
                break;
            case GeneralConfigDefine::Right:
                pos = LingmoUIQuick::Types::Right;
                break;
        }

        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("position"), pos);
        }
        Q_EMIT panelLocationChanged(m_panelLocation);
    }
}

void RemoteConfig::setPanelSizePolicy(GeneralConfigDefine::PanelSizePolicy panelSizePolicy)
{
    if (panelSizePolicy != m_panelSizePolicy) {
        m_panelSizePolicy = panelSizePolicy;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("panelSizePolicy"), panelSizePolicy);
        }
        Q_EMIT panelSizePolicyChanged(m_panelSizePolicy);
    }
}

void RemoteConfig::setPanelAutoHide(bool panelAutoHide)
{
    if (panelAutoHide != m_panelAutoHide) {
        m_panelAutoHide = panelAutoHide;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("panelAutoHide"), panelAutoHide);
        }
        Q_EMIT panelAutoHideChanged(m_panelAutoHide);
    }
}

void RemoteConfig::setPanelLock(bool panelLock)
{
    if (panelLock != m_panelLock) {
        m_panelLock = panelLock;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("lockPanel"), panelLock);
        }
        Q_EMIT panelLockChanged(m_panelLock);
    }
}

void RemoteConfig::setPanelWidgets(QStringList panelWidgets)
{
    if (panelWidgets != m_panelWidgets) {
        m_panelWidgets = panelWidgets;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("widgets"), panelWidgets);
        }
        Q_EMIT panelWidgetsChanged(m_panelWidgets);
    }
}

void RemoteConfig::setTrayIconsInhibited(QStringList trayIconsInhibited)
{
    if (trayIconsInhibited != m_trayIconsInhibited) {
        m_trayIconsInhibited = trayIconsInhibited;
        m_systemTrayConfig->setValue(QStringLiteral("trayIconsInhibited"), trayIconsInhibited);
        Q_EMIT trayIconsInhibitedChanged(m_trayIconsInhibited);
    }
}

void RemoteConfig::onPanelViewConfigChanged(const QString &key)
{
    if (key == QStringLiteral("disabledWidgets")) {
        m_disabledWidgets = m_mainViewConfig->getValue(key).toStringList();
        Q_EMIT disabledWidgetsChanged(m_disabledWidgets);
    } else if(key == "panelSizePolicy") {
        auto value = m_mainViewConfig->getValue(QStringLiteral("panelSizePolicy")).value<GeneralConfigDefine::PanelSizePolicy>();
        if(value != m_panelSizePolicy) {
            m_panelSizePolicy = value;
            Q_EMIT panelSizePolicyChanged(m_panelSizePolicy);
        }
    } else if (key == "panelAutoHide") {
        bool value = m_mainViewConfig->getValue(QStringLiteral("panelAutoHide")).toBool();
        if(value != m_panelAutoHide) {
            m_panelAutoHide = value;
            Q_EMIT panelAutoHideChanged(m_panelAutoHide);
        }
    } else if (key == "lockPanel") {
        bool value = m_mainViewConfig->getValue(QStringLiteral("lockPanel")).toBool();
        if(value != m_panelLock) {
            m_panelLock = value;
            Q_EMIT panelLockChanged(m_panelLock);
        }
    } else if (key == "widgets") {
        QStringList value = m_mainViewConfig->getValue(QStringLiteral("widgets")).toStringList();
        if(value != m_panelWidgets) {
            m_panelWidgets = value;
            Q_EMIT panelWidgetsChanged(m_panelWidgets);
        }
    } else if (key == "position") {
        updatePanelLocation();
        Q_EMIT panelLocationChanged(m_panelLocation);
    } else if (key == "showSystemTrayOnAllPanels") {
        bool value = m_mainViewConfig->getValue(QStringLiteral("showSystemTrayOnAllPanels")).toBool();
        if(value != m_showSystemTrayOnAllPanels) {
            m_showSystemTrayOnAllPanels = value;
            Q_EMIT showSystemTrayOnAllPanelsChanged(m_showSystemTrayOnAllPanels);
        }
    }
}

void RemoteConfig::onPanelConfigChanged(const QString &key)
{
    if (key == "showPanelOnAllScreens") {
        bool value = m_panelConfig->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
        if(value != m_showPanelOnAllScreens) {
            m_showPanelOnAllScreens = value;
            Q_EMIT showPanelOnAllScreensChanged(m_showPanelOnAllScreens);
        }
    }
}

void RemoteConfig::onTaskManagerConfigChanged(const QString &key)
{
    if(key == "mergeIcons") {
        auto value = m_taskManagerConfig->getValue(QStringLiteral("mergeIcons")).value<GeneralConfigDefine::MergeIcons>();
        if(value != m_mergeIcons) {
            m_mergeIcons = value;
            Q_EMIT mergeIconsChanged(m_mergeIcons);
        }
    } else if (key == "taskBarIconsShowedOn") {
        auto value = m_taskManagerConfig->getValue(QStringLiteral("taskBarIconsShowedOn")).value<GeneralConfigDefine::TaskBarIconsShowedOn>();
        if(value != m_taskBarIconsShowedOn) {
            m_taskBarIconsShowedOn = value;
            Q_EMIT taskBarIconsShowedOnChanged(m_taskBarIconsShowedOn);
        }
    }
}

void RemoteConfig::onSystemTrayConfigChanged(const QString &key)
{
    if(key == "trayIconsInhibited") {
        auto value = m_systemTrayConfig->getValue(QStringLiteral("trayIconsInhibited")).toStringList();
        if(value != m_trayIconsInhibited) {
            m_trayIconsInhibited = value;
            Q_EMIT trayIconsInhibitedChanged(m_trayIconsInhibited);
        }
    }
}

void RemoteConfig::updatePanelLocation()
{
    auto pos = m_mainViewConfig->getValue(QStringLiteral("position")).value<LingmoUIQuick::Types::Pos>();
    switch (pos) {
        case LingmoUIQuick::Types::Left:
            m_panelLocation = GeneralConfigDefine::PanelLocation::Left;
            break;
        case LingmoUIQuick::Types::Top:
            m_panelLocation = GeneralConfigDefine::PanelLocation::Top;
            break;
        case LingmoUIQuick::Types::Right:
            m_panelLocation = GeneralConfigDefine::PanelLocation::Right;
            break;
        default:
        case LingmoUIQuick::Types::Bottom:
            m_panelLocation = GeneralConfigDefine::PanelLocation::Bottom;
            break;
    }
}

QStringList RemoteConfig::disabledWidgets() const
{
    return m_disabledWidgets;
}

void RemoteConfig::disableWidget(const QString &id, bool disable)
{
    if (!m_mainViewConfig) {
        return;
    }

    if (disable) {
        if (m_disabledWidgets.contains(id)) {
            return;
        }
        m_disabledWidgets.append(id);

    } else if (m_disabledWidgets.removeAll(id) == 0) {
        return;
    }

    m_mainViewConfig->setValue(QStringLiteral("disabledWidgets"), m_disabledWidgets);
}

void RemoteConfig::setTaskBarIconsShowedOn(GeneralConfigDefine::TaskBarIconsShowedOn taskBarIconsShowedOn)
{
    if (taskBarIconsShowedOn != m_taskBarIconsShowedOn) {
        m_taskBarIconsShowedOn = taskBarIconsShowedOn;
        if(m_taskManagerConfig) {
            m_taskManagerConfig->setValue(QStringLiteral("taskBarIconsShowedOn"), taskBarIconsShowedOn);
        }
        Q_EMIT taskBarIconsShowedOnChanged(m_taskBarIconsShowedOn);
    }
}

void RemoteConfig::setShowPanelOnAllScreens(bool showPanelOnAllScreens)
{
    if (showPanelOnAllScreens != m_showPanelOnAllScreens) {
        m_showPanelOnAllScreens = showPanelOnAllScreens;
        if(m_panelConfig) {
            m_panelConfig->setValue(QStringLiteral("showPanelOnAllScreens"), showPanelOnAllScreens);
        }
        Q_EMIT showPanelOnAllScreensChanged(m_showPanelOnAllScreens);
    }
}

void RemoteConfig::setShowSystemTrayOnAllPanels(bool showSystemTrayOnAllPanels)
{
    if (showSystemTrayOnAllPanels != m_showSystemTrayOnAllPanels) {
        m_showSystemTrayOnAllPanels = showSystemTrayOnAllPanels;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("showSystemTrayOnAllPanels"), showSystemTrayOnAllPanels);
        }
        Q_EMIT showSystemTrayOnAllPanelsChanged(m_showSystemTrayOnAllPanels);
    }
}
