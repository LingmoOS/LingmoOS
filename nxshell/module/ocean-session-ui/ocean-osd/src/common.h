// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_H
#define COMMON_H

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QDebug>

static const int ImageTextItemWidth = 338;
static const int ImageTextItemHeight = 60;
static const int TextItemWidth = 120;
static const int TextItemHeight = 40;

/*!
 * \~chinese \class DrawHelper
 * \~chinese \brief osd辅助绘图类
 */
class DrawHelper {
public:
    // 绘制图片
    static void DrawImage(QPainter *painter, const QStyleOptionViewItem &option, const QString &iconName, bool isLightTheme);
    //绘制文字
    static void DrawText(QPainter *painter, const QStyleOptionViewItem &option, const QString &text, QColor color = Qt::black, bool withImage = true);
    //绘制进度条
    static void DrawProgressBar(QPainter *painter, const QStyleOptionViewItem &option, double progress, const QColor color);
    // 绘制百分比数据
    static void DrawPercentValue(QPainter *painter, const QStyleOptionViewItem &option, double value, const QColor &color);
    // 绘制背景
    static void DrawBackground(QPainter *painter, const QStyleOptionViewItem &option);
};

#endif // COMMON_H
