// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadtrayplugins.h"
#include "environments.h"

#include <signal.h>

#include <DConfig>

#include <QDir>
#include <QTimer>
#include <QGuiApplication>

namespace dock {

LoadTrayPlugins::LoadTrayPlugins(QObject *parent)
    : QObject(parent)
{

}

LoadTrayPlugins::~LoadTrayPlugins()
{
    for (auto &pInfo : m_processes) {
        if (pInfo.process) {
            pInfo.process->kill();
            //pInfo.process->waitForFinished();
            pInfo.process->deleteLater();
        }
    }
}

void LoadTrayPlugins::loadDockPlugins()
{
    QString validExePath = loaderPath();
    if (validExePath.isEmpty()) {
        qWarning() << "No valid loader executable path found.";
        return;
    }

    auto pluginGroupMap = groupPlugins(allPluginPaths());
    for (auto it = pluginGroupMap.begin(); it != pluginGroupMap.end(); ++it) {
        if (it.value().isEmpty()) continue;
        qDebug() << "Load plugin:" << it.value() << " group:" << it.key();
        startProcess(validExePath, it.value(), it.key());
    }
}

void LoadTrayPlugins::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    auto *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    if (exitCode == SIGKILL || exitCode == SIGTERM || exitStatus != QProcess::CrashExit) return;

    for (auto it = m_processes.begin(); it != m_processes.end(); ++it) {
        if (it->process == process) {
            if (it->retryCount < m_maxRetries) {
                it->retryCount++;
                qWarning() << "Plugin exit:" << it->pluginPath << " code:" << exitCode << " exitStatus:" << exitStatus;
                QTimer::singleShot(1000, process, [ this, process ] {
                    setProcessEnv(process);
                    process->start();
                });
            } else {
                qWarning() << "Maximum retries reached for plugin:" << it->pluginPath;
                process->deleteLater();
                m_processes.erase(it);
            }
            break;
        }
    }
}

void LoadTrayPlugins::startProcess(const QString &loaderPath, const QString &pluginPath, const QString &groupName)
{
    auto *process = new QProcess(this);
    setProcessEnv(process);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &LoadTrayPlugins::handleProcessFinished);

    ProcessInfo pInfo = { process, pluginPath, 0 };
    m_processes.append(pInfo);

    process->setProgram(loaderPath);
    process->setArguments({"-p", pluginPath, "-g", groupName, "-platform", "wayland"});
    process->start();
}

void LoadTrayPlugins::setProcessEnv(QProcess *process)
{
    if (!process) return;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // TODO: use protocols to determine the environment instead of environment variables
    env.remove("OCEAN_CURRENT_COMPOSITOR");

    process->setProcessEnvironment(env);
}

QString LoadTrayPlugins::loaderPath() const
{
    QStringList execPaths;
    execPaths << qEnvironmentVariable("TRAY_LOADER_EXECUTE_PATH")
              << QString("%1/trayplugin-loader").arg(CMAKE_INSTALL_FULL_LIBEXECDIR);

    QString validExePath;
    for (const QString &execPath : execPaths) {
        if (QFile::exists(execPath)) {
            validExePath = execPath;
            break;
        }
    }

    return validExePath;
}

QStringList LoadTrayPlugins::allPluginPaths() const
{
    QStringList dirs;
    const auto pluginsPath = qEnvironmentVariable("TRAY_DEBUG_PLUGIN_PATH");
    if (!pluginsPath.isEmpty())
        dirs << pluginsPath.split(QDir::listSeparator());

    if (dirs.isEmpty())
        dirs << pluginDirs;

    QStringList pluginPaths;
    for (auto &pluginDir : dirs) {
        QDir dir(pluginDir);
        if (!dir.exists()) {
            qWarning() << "The plugin directory does not exist:" << pluginDir;
            continue;
        }

        auto pluginFileInfos = dir.entryInfoList({"*.so"}, QDir::Files);
        for (auto &pluginInfo : pluginFileInfos) {
            pluginPaths.append(pluginInfo.absoluteFilePath());
        }
    }

    return pluginPaths;
}

QMap<QString, QString> LoadTrayPlugins::groupPlugins(const QStringList &pluginPaths) const
{
    const QString selfMaintenancePluginsKey = "selfMaintenanceTrayPlugins";
    const QString subprojectPluginsKey = "subprojectTrayPlugins";
    const QString crashPronePluginsKey = "crashProneTrayPlugins";
    const QString otherPluginsKey = "otherTrayPlugins";

    auto dConfig = Dtk::Core::DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ds.dock.tray", QString());
    QStringList selfMaintenanceTrayPlugins = dConfig->value(selfMaintenancePluginsKey).toStringList();
    QStringList subprojectTrayPlugins = dConfig->value(subprojectPluginsKey).toStringList();
    QStringList crashProneTrayPlugins = dConfig->value(crashPronePluginsKey).toStringList();
    dConfig->deleteLater();

    QStringList selfMaintenancePluginPaths;
    QStringList subprojectPluginPaths;
    QStringList crashPronePluginPaths;
    QStringList otherPluginPaths;

    for (auto &filePath : pluginPaths) {
        QString pluginName = filePath.section("/", -1);
        if (crashProneTrayPlugins.contains(pluginName)) {
            crashPronePluginPaths.append(filePath);
        } else if (selfMaintenanceTrayPlugins.contains(pluginName)) {
            selfMaintenancePluginPaths.append(filePath);
        } else if (subprojectTrayPlugins.contains(pluginName)) {
            subprojectPluginPaths.append(filePath);
        } else {
            otherPluginPaths.append(filePath);
        }
    }

    QMap<QString, QString> pluginGroup;

    if (!selfMaintenancePluginPaths.isEmpty()) {
        pluginGroup.insert(selfMaintenancePluginsKey, selfMaintenancePluginPaths.join(";"));
    }

    if (!subprojectPluginPaths.isEmpty()) {
        pluginGroup.insert(subprojectPluginsKey, subprojectPluginPaths.join(";"));
    }

    if (!crashPronePluginPaths.isEmpty()) {
        pluginGroup.insert(crashPronePluginsKey, crashPronePluginPaths.join(";"));
    }

    if (!otherPluginPaths.isEmpty()) {
        pluginGroup.insert(otherPluginsKey, otherPluginPaths.join(";"));
    }

    return pluginGroup;
}
}
