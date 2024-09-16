// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadtrayplugins.h"
#include "environments.h"

#include <signal.h>

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
    QStringList filters;
    filters << "*.so";

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

    if (validExePath.isEmpty()) {
        qWarning() << "No valid loader executable path found.";
        return;
    }

    QStringList dirs;
    const auto pluginsPath = qEnvironmentVariable("TRAY_DEBUG_PLUGIN_PATH");
    if (!pluginsPath.isEmpty())
        dirs << pluginsPath.split(QDir::listSeparator());

    if (dirs.isEmpty())
        dirs << pluginDirs;

    for (auto &pluginDir : dirs) {
        QDir dir(pluginDir);
        if (!dir.exists()) {
            qWarning() << "The plugin directory does not exist:" << pluginDir;
            continue;
        }

        auto pluginFileInfos = dir.entryInfoList(filters, QDir::Files);
        foreach (auto pluginInfo, pluginFileInfos) {
            qInfo() << "pluginLoader load plugin" << pluginInfo.absoluteFilePath();

            QProcess *process = new QProcess(this);
            setProcessEnv(process);

            connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, &LoadTrayPlugins::handleProcessFinished);

            ProcessInfo pInfo = { process, pluginInfo.absoluteFilePath(), 0 };
            m_processes.append(pInfo);

            process->setProgram(validExePath);
            process->setArguments({"-p", pluginInfo.absoluteFilePath(), "-platform", "wayland"});
            process->start();
        }
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

void LoadTrayPlugins::setProcessEnv(QProcess *process)
{
    if (!process) return;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("QT_SCALE_FACTOR", QString::number(qApp->devicePixelRatio()));
    env.insert("D_DXCB_DISABLE_OVERRIDE_HIDPI", "1");

    process->setProcessEnvironment(env);
}

}
