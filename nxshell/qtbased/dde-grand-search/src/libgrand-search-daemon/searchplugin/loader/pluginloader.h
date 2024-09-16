// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "utils/searchplugininfo.h"

#include <QObject>
#include <QMap>

namespace GrandSearch {

class PluginLoader : public QObject
{
    Q_OBJECT
public:
    explicit PluginLoader(QObject *parent = nullptr);
    void setPluginPath(const QStringList &dirPaths);
    bool load();
    QList<SearchPluginInfo> plugins() const;
    bool getPlugin(const QString &name, SearchPluginInfo &plugin) const;

protected:
    bool readInfo(const QString &path, SearchPluginInfo &info);
private:
    QStringList m_paths;
    QMap<QString, SearchPluginInfo> m_plugins;
};

}

#endif // PLUGINLOADER_H
