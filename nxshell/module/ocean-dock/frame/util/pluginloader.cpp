// Copyright (C) 2011 ~ 2018 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pluginloader.h"

#include <QDir>
#include <QDebug>
#include <QLibrary>
#include <QGSettings>

#include <LSysInfo>

DCORE_USE_NAMESPACE

PluginLoader::PluginLoader(const QString &pluginDirPath, QObject *parent)
    : QThread(parent)
    , m_pluginDirPath(pluginDirPath)
{
}

void PluginLoader::run()
{
    QDir pluginsDir(m_pluginDirPath);
    const QStringList files = pluginsDir.entryList(QDir::Files);

    auto getDisablePluginList = [ = ] {
        if (QGSettings::isSchemaInstalled("com.lingmo.ocean.dock.disableplugins")) {
            QGSettings gsetting("com.lingmo.ocean.dock.disableplugins", "/com/lingmo/ocean/dock/disableplugins/");
            return gsetting.get("disable-plugins-list").toStringList();
        }
        return QStringList();
    };

    const QStringList disable_plugins_list = getDisablePluginList();

    QStringList plugins;

    // 查找可用插件
    for (QString file : files) {
        if (!QLibrary::isLibrary(file))
            continue;

        // 社区版需要加载键盘布局，其他不需要
        if (file.contains("libkeyboard-layout") && !LSysInfo::isCommunityEdition())
            continue;

        // TODO: old dock plugins is uncompatible
        if (file.startsWith("libocean-dock-"))
            continue;

        if (disable_plugins_list.contains(file)) {
            qDebug() << "disable loading plugin:" << file;
            continue;
        }

        plugins << file;
    }

    for (auto plugin : plugins) {
        emit pluginFound(pluginsDir.absoluteFilePath(plugin));
    }

    emit finished();
}
