// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSTANDARDPATHS_H
#define DFMSTANDARDPATHS_H

#include <QString>
#include <QStandardPaths>
#include <QUrl>

class DUrl;
class DFMStandardPaths
{
public:
    enum StandardLocation {
        TrashPath,   // 回收站路径
        TrashFilesPath,   // 回收站文件存放地址
        TrashInfosPath,   // 回收站文件信息路径
        TranslationPath,   // 翻译文件所在路径
        TemplatesPath,   // 临时文件所在路径
        MimeTypePath,   // mimetype文件的路径
        PluginsPath,   // 插件路径
        ExtensionsPath,
        ExtensionsAppEntryPath,
        ThumbnailPath,   // 缩略图路径
        ThumbnailFailPath,   // 缩略图s失败路径
        ThumbnailLargePath,   // 缩略图大图片路径
        ThumbnailNormalPath,   // 缩略图普通大小路径
        ThumbnailSmallPath,   // 缩略图小图片路径
        ApplicationConfigPath,   // app配置文件路径 ~/.config
        ApplicationSharePath,   // app共享路径 /usr/share/dde-file-manager
        RecentPath,   // 最近使用的文件保存路径
        HomePath,   // home目录的文件路径
        DesktopPath,   // 桌面目录路径
        VideosPath,   // 视频目录路径
        MusicPath,   // 音乐目录路径
        PicturesPath,   // 图片目录路径
        DocumentsPath,   // 文档目录路径
        DownloadsPath,   // 下载目录路径
        CachePath,   // 缓存文件路径 ~/.cache/dde-file-manager
        DiskPath,   // 磁盘路径
        NetworkRootPath,   // 网络邻居路径
        UserShareRootPath,   // 用户共享的根目录路径
        ComputerRootPath,   // 计算机根目录路径
        Root,   // 根目录路径
        Vault   // 保险箱路径
    };

    static QString location(StandardLocation type);
#ifdef QMAKE_TARGET
    static QString getConfigPath();
#endif
    static QString getCachePath();

private:
    DFMStandardPaths();
};

#endif // DFMSTANDARDPATHS_H
