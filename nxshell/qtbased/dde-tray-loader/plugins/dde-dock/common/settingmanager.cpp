// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingmanager.h"
#include "../util/utils.h"

#include <DConfig>

#include <QGSettings>
#include <QMetaMethod>
#include <QMetaObject>

DCORE_USE_NAMESPACE

static DConfig *dockConfig = nullptr;
static DConfig *quickPanelConfig = nullptr;

const static QString DCONFIG_DOCK_PLUGINS = QStringLiteral("dockedQuickPlugins");
const static QString DCONFIG_SHOW_WINDOW_NAME = QStringLiteral("showWindowName");
const static QString DCONFIG_ENABLE_SAFE_MODE = QStringLiteral("enableSafeMode");
const static QString DCONFIG_DELAY_INTERVAL_ON_HIDE = QStringLiteral("delayIntervalOnHide");
const static QString DCONFIG_TOGGLE_DESKTOP_INTERVAL = QStringLiteral("toggleDesktopInterval");
const static QString DCONFIG_ALWAYS_HIDE_DOCK = QStringLiteral("alwaysHideDock");
const static QString DCONFIG_ENABLE_SHOWDESKTOP = QStringLiteral("enableShowDesktop");

SettingManager::SettingManager(QObject *parent)
    : QObject(parent)
    , m_enableSafeMode(true)
    , m_delayIntervalOnHide(0)
    , m_showWindowName(-1)
    , m_toggleDesktopInterval(0)
    , m_alwaysHideDock(false)
    , m_enableShowDesktop(true)
{
    if (!dockConfig)
        dockConfig = DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.dock", QString(), this);

    if (!quickPanelConfig)
        quickPanelConfig = DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.dock.plugin.quick-panel", QString(), this);

    if (dockConfig) {
        connect(dockConfig, &DConfig::valueChanged, this, &SettingManager::onDockConfigChanged);
        for (const auto &key : dockConfig->keyList()) {
            if (DCONFIG_ENABLE_SAFE_MODE == key)
                m_enableSafeMode = dockConfig->value(DCONFIG_ENABLE_SAFE_MODE).toBool();
            else if (DCONFIG_DELAY_INTERVAL_ON_HIDE == key)
                m_delayIntervalOnHide = dockConfig->value(DCONFIG_DELAY_INTERVAL_ON_HIDE).toInt();
            else if (DCONFIG_SHOW_WINDOW_NAME == key)
                m_showWindowName = dockConfig->value(DCONFIG_SHOW_WINDOW_NAME).toInt();
            else if (DCONFIG_TOGGLE_DESKTOP_INTERVAL == key)
                m_toggleDesktopInterval = dockConfig->value(DCONFIG_TOGGLE_DESKTOP_INTERVAL).toInt();
            else if (DCONFIG_ALWAYS_HIDE_DOCK == key)
                m_alwaysHideDock = dockConfig->value(DCONFIG_ALWAYS_HIDE_DOCK).toBool();
            else if (DCONFIG_ENABLE_SHOWDESKTOP == key)
                m_enableShowDesktop = dockConfig->value(DCONFIG_ENABLE_SHOWDESKTOP).toBool();
        }
    }

    if (quickPanelConfig) {
        connect(quickPanelConfig, &DConfig::valueChanged, this, &SettingManager::onQuickPanelConfigChanged);

        m_dockedQuickPlugins = quickPanelConfig->value(DCONFIG_DOCK_PLUGINS, QStringList()).toStringList();
    }
}

SettingManager::~SettingManager()
{
}

SettingManager *SettingManager::instance()
{
    static SettingManager config;
    return &config;
}

bool SettingManager::enableSafeMode() const
{
    return m_enableSafeMode;
}

bool SettingManager::enableShowDesktop() const
{
    return m_enableShowDesktop;
}

int SettingManager::delayIntervalOnHide() const
{
    return m_delayIntervalOnHide;
}

int SettingManager::showWindowName() const
{
    return m_showWindowName;
}

int SettingManager::toggleDesktopInterval() const
{
    return m_toggleDesktopInterval;
}

bool SettingManager::alwaysHideDock() const
{
    return m_alwaysHideDock;
}

QStringList SettingManager::dockedPlugins() const
{
    return m_dockedQuickPlugins;
}

void SettingManager::setPluginDocked(const QString &pluginName, bool docked)
{
    if (!quickPanelConfig)
        return;

    if (docked && !m_dockedQuickPlugins.contains(pluginName))
        m_dockedQuickPlugins.append(pluginName);
    else if (!docked)
        m_dockedQuickPlugins.removeAll(pluginName);

    quickPanelConfig->setValue(DCONFIG_DOCK_PLUGINS, m_dockedQuickPlugins);
}

void SettingManager::onDockConfigChanged(const QString &key)
{
    if (key == DCONFIG_ENABLE_SAFE_MODE) {
        m_enableSafeMode = dockConfig->value(DCONFIG_ENABLE_SAFE_MODE).toBool();
    } else if (key == DCONFIG_DELAY_INTERVAL_ON_HIDE) {
        m_delayIntervalOnHide = dockConfig->value(DCONFIG_DELAY_INTERVAL_ON_HIDE).toInt();
    } else if (key == DCONFIG_SHOW_WINDOW_NAME) {
        m_showWindowName = dockConfig->value(DCONFIG_SHOW_WINDOW_NAME).toInt();
    } else if (key == DCONFIG_TOGGLE_DESKTOP_INTERVAL) {
        m_toggleDesktopInterval = dockConfig->value(DCONFIG_TOGGLE_DESKTOP_INTERVAL).toInt();
        emit toggleDesktopIntervalChanged(m_toggleDesktopInterval);
    } else if (key == DCONFIG_ALWAYS_HIDE_DOCK) {
        m_alwaysHideDock = dockConfig->value(DCONFIG_ALWAYS_HIDE_DOCK).toBool();
        emit alwaysHideDockChanged(m_alwaysHideDock);
    } else if (DCONFIG_ENABLE_SHOWDESKTOP == key)
        m_enableShowDesktop = dockConfig->value(DCONFIG_ENABLE_SHOWDESKTOP).toBool();
        emit enableShowDesktopChanged(m_enableShowDesktop);
}

void SettingManager::onQuickPanelConfigChanged(const QString &key)
{
    if (key == DCONFIG_DOCK_PLUGINS) {
        m_dockedQuickPlugins = quickPanelConfig->value(DCONFIG_DOCK_PLUGINS).toStringList();
        Q_EMIT dockedPluginsChanged(m_dockedQuickPlugins);
    }
}
