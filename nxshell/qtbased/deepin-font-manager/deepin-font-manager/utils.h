// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QHash>

/*************************************************************************
 <Class>         Utils
 <Description>   全文索引类
 <Author>
 <Note>          null
*************************************************************************/
class Utils : public QObject
{
public:
    static QHash<QString, QPixmap> m_imgCacheHash;
    static QHash<QString, QString> m_fontNameCache;

    //读取应用配置文件信息字符串
    static QString getConfigPath();
    //判断当前是否为MIME类型的数据库
    static bool isFontMimeType(const QString &filePath);
    //获取文件后缀类型
    static QString suffixList();
    //读取图像信息
    static QPixmap renderSVG(const QString &filePath, const QSize &size);

    static QString convertToPreviewString(const QString &fontFilePath, const QString &srcString);

    //判断是否wayland环境
    static bool isWayland();

    //清除图像缓存
    static void clearImgCache();
};

#endif
