// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginloader.h"

#include <QDir>
#include <QDebug>
#include <QLibrary>
#include <QApplication>

#include <DSysInfo>

DCORE_USE_NAMESPACE

PluginLoader::PluginLoader(QObject *parent)
    : QThread(parent)
{
}

void PluginLoader::run()
{
    QString pluginsDirPath(qApp->applicationDirPath() + "/plugins");
#ifndef QT_DEBUG
    pluginsDirPath = "/usr/lib/dde-launcher/plugins";
#endif

    QDir pluginsDir(pluginsDirPath);

    const QStringList files = pluginsDir.entryList(QDir::Files);
    QStringList plugins;

    // 查找可用插件
    for (QString file : files) {
        if (!QLibrary::isLibrary(file))
            continue;

        plugins << file;
    }

    for (auto plugin : plugins) {
        emit pluginFounded(pluginsDir.absoluteFilePath(plugin));
    }

    emit finished();
}
