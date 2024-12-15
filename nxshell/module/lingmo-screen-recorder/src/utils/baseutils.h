// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <QCursor>
#include <QFont>
#include <QLayout>
#include <QFontMetrics>
#include <QPainter>

enum ResizeDirection {
    Rotate,
    Moving,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Top,
    Bottom,
    Left,
    Right,
    Outting,
};

class BaseUtils : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 颜色按钮的枚举
     * 注：此处的枚举名称应与工程目录的图片名称保持一致(/assets/image/color_pen/~.svg)
     */
    enum ButtonColors {
        Black = 0,   //#000000 黑色
        Gray,        //#7D7D7D 灰色
        White,       //#FFFFFF 白色
        Red,         //#F82A2A 红色
        Orange,      //#FF8100 橙色
        Yellow,      //#FFF100 黄色
        LightLime,   //#CDFF00 亮柠檬绿
        Aquamarine,  //#1EE9A8 碧蓝色
        Green,       //#006D06 绿色
        Blue,        //#0089F7 蓝色
        Purple,      //#7600AC 紫色
        DarkBlue     //#0C00A0 深蓝
    };
    Q_ENUM(ButtonColors)
public:
    static QCursor setCursorShape(QString cursorName, int colorIndex = 0);
    static int stringWidth(const QFont &f, const QString &str);
    /**
     * @brief 根据当前形状的颜色索引获取当前形状的颜色
     * @param index:颜色索引
     * @return 索引对应的颜色
     */
    static QColor       colorIndexOf(int index);
    static int                colorIndex(QColor color);
    /**
     * @brief 判断后缀是否是"bmp" "jpg" "jpeg" "png" "pbm" "pgm" "xbm" "xpm"
     * @param suffix:后缀
     * @return true:是 false:否
     */
    static bool          isValidFormat(QString suffix);
    static bool          isCommandExist(QString command);
    static QMap<QString, QCursor> m_shapesCursor;
};
#endif // BASEUTILS_H
