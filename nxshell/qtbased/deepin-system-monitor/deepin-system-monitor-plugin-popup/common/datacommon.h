// Copyright (C) 2018 ~ 2025 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATACOMMON_H
#define DATACOMMON_H


#include <QSize>
#include <QPixmap>
#include <QPainter>
#include <QPalette>
#include <QBitmap>
#include <QTimer>
#include <QIcon>

#define MAX(a,b) ((a) > (b) ? (a):(b))
#define MIN(a,b) ((a) < (b) ? (a):(b))


static const QString DBusClipBoardService = "com.deepin.dde.Clipboard";
static const QString DBusClipBoardPath = "/com/deepin/dde/Clipboard";

typedef struct {
    QStringList cornerIconList;
    QIcon fileIcon;
} FileIconData;

namespace  Globals {

const QString SERVICE_NAME = "com.deepin.SystemMonitorPluginPopup";
const QString SERVICE_PATH = "/com/deepin/SystemMonitorPluginPopup";

const static int WindowWidth = 300;
const static int WindowMargin = 10;             //边距
const static int DockMargin = 5;   //dock栏边距
const static int WindowTitleHeight = 56;
const static int ItemTitleHeight = 36;          //Item标题栏高度
const static int ItemStatusBarHeight = 30;      //Item状态栏高度
const static int ItemWidth = WindowWidth - 2 * WindowMargin;
const static int ItemHeight = 200;
const static int ItemMargin = 10;
const static int PixmapWidth = 180;             //图像最大显示宽度
const static int PixmapHeight = 100;            //图像最大显示高度
const static int FileIconWidth = PixmapWidth;
const static int FileIconHeight = PixmapHeight;
const static int PixmapxStep = 15;
const static int PixmapyStep = 5;
const static int ContentMargin = 21;
const static int TextContentTopMargin = 20;
const static int AnimationTime = 300;           //ms

//浅色透明度
const static int TitleTransLight = 75;
const static int contentTransLight = 50;
const static int hoverTransLight = 75;
//深色透明度
const static int TitleTransDark = 20;
const static int contentTransDark = 13;
const static int hoverTransDark = 38;

//字体大小
const static int ContentFont = 14;
const static int PicFont = 12;
const static int ContentUnitFont = 9;
const static int subContentFont = 8;

const QString DEEPIN_SYSTEM_MONITOR_PATH = "/usr/bin/deepin-system-monitor";

/*!
 * \~chinese \name GetScale
 * \~chinese \brief 获取缩放的比例
 * \~chinese \param size 当前所需要的大小
 * \~chinese \param targetWidth  目标宽度
 * \~chinese \param targetHeight 目标高度
 */
static qreal GetScale(QSize size, int targetWidth, int targetHeight)
{
    qreal scale = 1.0;

    if (size.width() >= size.height() * (targetWidth * 1.0 / targetHeight)) {
        scale = size.width() * 1.0 / targetWidth;
    } else {
        scale = size.height() * 1.0 / targetHeight;
    }

    return scale;
}

/*!
 * \~chinese \name pixmapScaled
 * \~chinese \brief 图片缩放
 * \~chinese \param pixmap 源pixmap数据
 * \~chinese \return 返回一个宽度为180,高度为100的图片,并且缩放比例不变
 */
inline QPixmap pixmapScaled(const QPixmap &pixmap)
{
    if (pixmap.isNull())
        return pixmap;
    qreal scale = Globals::GetScale(pixmap.size(), PixmapWidth, PixmapHeight);
    return pixmap.scaled(pixmap.size() / scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

/*!
 * \~chinese \name GetRoundPixmap
 * \~chinese \brief 获取一张带有圆角的图片，边框颜色为系统调色板中的Base颜色（加透明度）
 * \~chinese \param pix 源pixmap数据
 * \~chinese \param borderColor 边框颜色
 * \~chinese \return 如果pix为空返回一个空的图片,如果不为空返回调整后的图片.
 */
inline QPixmap GetRoundPixmap(const QPixmap &pix, QColor borderColor)
{
    if (pix.isNull()) {
        return QPixmap();
    }

    int radius = 10;
    int borderWidth = 10;
//    if (pix.width() > pix.height()) {
//        radius = int(8 * 1.0 / PixmapWidth * pix.width());
//        borderWidth = int(10 * 1.0 / PixmapWidth * pix.width());
//    } else {
//        radius = int(8 * 1.0 / PixmapHeight * pix.height());
//        borderWidth = int(10 * 1.0 / PixmapHeight * pix.height());
//    }

    QPixmap pixmap = pix;
    //绘制边框
    QPainter pixPainter(&pixmap);
    pixPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    borderColor.setAlpha(60);
    pixPainter.setPen(QPen(borderColor, borderWidth));
    pixPainter.setBrush(Qt::transparent);
    pixPainter.drawRoundedRect(pixmap.rect(), radius, radius);

    //设置圆角
    QSize size(pixmap.size());
    QBitmap mask(size);
    QPainter painter(&mask);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.fillRect(mask.rect(), Qt::color0);
    painter.setBrush(Qt::color1);
    painter.drawRoundedRect(mask.rect(), radius, radius);

    //遮罩
    QPixmap image = pixmap;
    image.setMask(mask);
    return image;
}
} ;
#endif // DATACOMMON_H
