// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWPLUGINMANAGER_H
#define PREVIEWPLUGINMANAGER_H

#include "gui/datadefine.h"
#include "global/matcheditem.h"

namespace GrandSearch {

class PreviewPlugin;

class PreviewPluginManager : QObject
{
public:
    explicit PreviewPluginManager();
    ~PreviewPluginManager();

    PreviewPlugin *getPreviewPlugin(const MatchedItem& item);
    static bool isMimeTypeMatch(const QString &mimetype, const QStringList &supportMimeTypes);
private:
    void clearPluginInfo();
    bool readPluginConfig();
    bool loadConfig();
    bool readInfo(const QString &path, PreviewPluginInfo &info);

    PreviewPluginInfo *getPreviewPlugin(const QString &name);
    void setPluginPath(const QStringList &dirPaths);
    // 插件版本是否向下兼容
    bool downwardCompatibility(const QString &version);

private:
    QStringList m_paths;
    PreviewPluginInfoList m_plugins;
    QString m_mainVersion;
};

}

#endif // PREVIEWPLUGINMANAGER_H
