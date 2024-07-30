/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modulemanager.h"

#include <PulseAudioQt/Module>
#include <PulseAudioQt/Server>

#include "gsettingsitem.h"
#define PA_SETTINGS_PATH_MODULES "/org/freedesktop/pulseaudio/module-groups"

#include <QTimer>
#include <chrono>

using namespace std::chrono_literals;

namespace PulseAudioQt
{
class ConfigModule : public GSettingsItem
{
public:
    ConfigModule(const QString &configName, const QString &moduleName, QObject *parent);
    bool isEnabled() const;
    void setEnabled(bool enabled, const QVariant &args = QVariant());

private:
    QString m_moduleName;
};

ConfigModule::ConfigModule(const QString &configName, const QString &moduleName, QObject *parent)
    : GSettingsItem(QStringLiteral(PA_SETTINGS_PATH_MODULES "/") + configName + QStringLiteral("/"), parent)
    , m_moduleName(moduleName)
{
}

bool ConfigModule::isEnabled() const
{
    return value(QStringLiteral("enabled")).toBool();
}

void ConfigModule::setEnabled(bool enabled, const QVariant &args)
{
    set(QStringLiteral("locked"), true);

    if (enabled) {
        set(QStringLiteral("name0"), m_moduleName);
        set(QStringLiteral("args0"), args);
        set(QStringLiteral("enabled"), true);
    } else {
        set(QStringLiteral("enabled"), false);
    }
    set(QStringLiteral("locked"), false);
}

ModuleManager::ModuleManager(QObject *parent)
    : QObject(parent)
{
    m_combineSinks = new ConfigModule(QStringLiteral("combine"), QStringLiteral("module-combine-sink"), this);
    m_switchOnConnect = new ConfigModule(QStringLiteral("switch-on-connect"), QStringLiteral("module-switch-on-connect"), this);
    m_deviceManager = new ConfigModule(QStringLiteral("device-manager"), QStringLiteral("module-device-manager"), this);

    connect(m_combineSinks, &ConfigModule::subtreeChanged, this, &ModuleManager::combineSinksChanged);
    connect(m_switchOnConnect, &ConfigModule::subtreeChanged, this, &ModuleManager::switchOnConnectChanged);
    connect(m_deviceManager, &ConfigModule::subtreeChanged, this, &ModuleManager::switchOnConnectChanged);

    connect(Context::instance()->server(), &Server::updated, this, &ModuleManager::serverUpdated);

    auto *updateModulesTimer = new QTimer(this);
    updateModulesTimer->setInterval(500ms);
    updateModulesTimer->setSingleShot(true);
    connect(updateModulesTimer, &QTimer::timeout, this, &ModuleManager::updateLoadedModules);
    connect(Context::instance(), &Context::moduleAdded, updateModulesTimer, static_cast<void (QTimer::*)(void)>(&QTimer::start));
    connect(Context::instance(), &Context::moduleRemoved, updateModulesTimer, static_cast<void (QTimer::*)(void)>(&QTimer::start));
    updateLoadedModules();
}

ModuleManager::~ModuleManager() = default;
;

bool ModuleManager::settingsSupported() const
{
    // PipeWire does not (yet) have support for module-switch-on-connect and module-combine-sink
    // Also switching streams is the default there
    // TODO Check whether there is a PipeWire-specific way to do these
    return !Context::instance()->server()->isPipeWire();
}

bool ModuleManager::combineSinks() const
{
    return m_combineSinks->isEnabled();
}

void ModuleManager::setCombineSinks(bool combineSinks)
{
    m_combineSinks->setEnabled(combineSinks);
}

bool ModuleManager::switchOnConnect() const
{
    // switch on connect and device-manager do the same task. Only one should be enabled

    // Note on the first run m_deviceManager will appear to be disabled even though it's actually running
    // because there is no gconf entry, however m_switchOnConnect will only exist if set by Plasma PA
    // hence only check this entry
    return m_switchOnConnect->isEnabled();
}

void ModuleManager::setSwitchOnConnect(bool switchOnConnect)
{
    m_deviceManager->setEnabled(!switchOnConnect);
    m_switchOnConnect->setEnabled(switchOnConnect);
}

QStringList ModuleManager::loadedModules() const
{
    return m_loadedModules;
}

void ModuleManager::updateLoadedModules()
{
    m_loadedModules.clear();
    const auto modules = Context::instance()->modules();
    for (Module *module : modules) {
        m_loadedModules.append(module->name());
    }
    Q_EMIT loadedModulesChanged();
}

bool ModuleManager::configModuleLoaded() const
{
    return m_loadedModules.contains(configModuleName());
}

QString ModuleManager::configModuleName() const
{
    return QStringLiteral("module-gsettings");
}
}
