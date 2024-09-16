// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QSize>
#include <QPixmap>
#include <QPainter>
#include <QPalette>
#include <QBitmap>
#include <QTimer>
#include <QIcon>

#define MAX(a,b) ((a) > (b) ? (a):(b))
#define MIN(a,b) ((a) < (b) ? (a):(b))

inline constexpr int WindowWidth = 300;
inline constexpr int WindowMargin = 10;             //边距
inline constexpr int WindowTitleHeight = 56;
inline constexpr int ItemTitleHeight = 36;          //Item标题栏高度
inline constexpr int ItemStatusBarHeight = 30;      //Item状态栏高度
inline constexpr int ItemWidth = WindowWidth - 2 * WindowMargin;
inline constexpr int ItemHeight = 200;
inline constexpr int ItemMargin = 10;
inline constexpr int PixmapWidth = 180;             //图像最大显示宽度
inline constexpr int PixmapHeight = 100;            //图像最大显示高度
inline constexpr int FileIconWidth = PixmapWidth;
inline constexpr int FileIconHeight = PixmapHeight;
inline constexpr int PixmapxStep = 15;
inline constexpr int PixmapyStep = 5;
inline constexpr int ContentMargin = 21;
inline constexpr int TextContentTopMargin = 20;
inline constexpr int AnimationTime = 300;           //ms

static const QString DBusClipBoardService = "org.deepin.dde.Clipboard1";
static const QString DBusClipBoardPath = "/org/deepin/dde/Clipboard1";

static const QString TextUriListLiteral QStringLiteral("text/uri-list");
static const QString TextHtmlLiteral QStringLiteral("text/html");
static const QString TextPlainLiteral QStringLiteral("text/plain");
static const QString ApplicationXColorLiteral QStringLiteral("application/x-color");
static const QString ApplicationXQtImageLiteral QStringLiteral("application/x-qt-image");

typedef struct {
    QStringList cornerIconList;
    QIcon fileIcon;
} FileIconData;

namespace  Globals {
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
#endif // CONSTANTS_H
