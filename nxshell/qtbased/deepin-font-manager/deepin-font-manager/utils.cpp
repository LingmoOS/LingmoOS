// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "globaldef.h"

#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QTextLayout>
#include <QProcessEnvironment>

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

/*************************************************************************
 <Function>      getConfigPath
 <Description>   读取应用配置文件信息字符串
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QString Utils::getConfigPath()
{
    QDir dir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first())
             .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

/*************************************************************************
 <Function>      isFontMimeType
 <Description>   判断当前是否为MIME类型的数据库
 <Author>        null
 <Input>
    <param1>     filePath        Description:文件路径
 <Return>        bool            Description:是否为MIME类型的数据库，true，是；false：否
 <Note>          null
*************************************************************************/
bool Utils::isFontMimeType(const QString &filePath)
{
    const QString mimeName = QMimeDatabase().mimeTypeForFile(filePath).name();

    if (QString(FONT_FILE_MIME).contains(mimeName)) {
        return true;
    }

    return false;
}

/*************************************************************************
 <Function>      suffixList
 <Description>   获取文件后缀类型
 <Author>
 <Input>         null
 <Return>        QString            Description:文件后缀类型
 <Note>          null
*************************************************************************/
QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
}

/*************************************************************************
 <Function>      renderSVG
 <Description>   读取图像信息
 <Author>
 <Input>
    <param1>     filePath        Description:文件路径
    <param2>     size            Description:size大小
 <Return>        QPixmap         Description:返回读取到的图像信息
 <Note>          null
*************************************************************************/
QPixmap Utils::renderSVG(const QString &filePath, const QSize &size)
{
    if (m_imgCacheHash.contains(filePath)) {
        return m_imgCacheHash.value(filePath);
    }

    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    m_imgCacheHash.insert(filePath, pixmap);

    return pixmap;
}

/*************************************************************************
 <Function>      convertToPreviewString
 <Description>   转换成预览文本信息
 <Author>
 <Input>
    <param1>     fontFilePath    Description:字体路径
    <param3>     srcString       Description:字符串参数
 <Return>        QString         Description:预览文本信息
 <Note>          null
*************************************************************************/
QString Utils::convertToPreviewString(const QString &fontFilePath, const QString &srcString)
{
    if (fontFilePath.isEmpty()) {
        return srcString;
    }

    QString strFontPreview = srcString;

    QRawFont rawFont(fontFilePath, 0, QFont::PreferNoHinting);
    bool isSupport = rawFont.supportsCharacter(QChar('a'));
    bool isSupportF = rawFont.supportsCharacter(QChar('a' | 0xf000));
    if ((!isSupport && isSupportF)) {
        QChar *chArr = new QChar[srcString.length() + 1];
        for (int i = 0; i < srcString.length(); i++) {
            int ch = srcString.at(i).toLatin1();
            //判断字符ascii在32～126范围内(共95个)
            if (ch >= 32 && ch <= 126) {
                ch |= 0xf000;
                chArr[i] = QChar(ch);
            } else {
                chArr[i] = srcString.at(i);
            }
        }
        chArr[srcString.length()] = '\0';
        QString strResult(chArr);
        strFontPreview = strResult;
        delete[] chArr;
    }

    return strFontPreview;
}

bool Utils::isWayland()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    } else {
        return false;
    }
}

void Utils::clearImgCache()
{
    m_imgCacheHash.clear();
}
