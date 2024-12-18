// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QProcess>

namespace dock {
const QStringList pluginDirs = {
        "/usr/lib/ocean-dock/plugins/",
        "/usr/lib/ocean-dock/plugins/quick-trays/",
        "/usr/lib/ocean-dock/plugins/system-trays/"
};

class LoadTrayPlugins : public QObject
{
    Q_OBJECT
public:
    explicit LoadTrayPlugins(QObject *parent = nullptr);
    ~LoadTrayPlugins() override;

    void loadDockPlugins();

private slots:
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString loaderPath() const;
    QStringList allPluginPaths() const;
    QMap<QString, QString> groupPlugins(const QStringList &pluginPaths) const;

    void startProcess(const QString &loaderPath, const QString &pluginPath, const QString &groupName);
    void setProcessEnv(QProcess *process);

private:
    struct ProcessInfo {
        QProcess *process = nullptr;
        QString pluginPath;
        int retryCount = 0;
    };

    QList<ProcessInfo> m_processes;
    const int m_maxRetries = 5;
};

}
