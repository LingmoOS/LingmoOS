// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAPESUTILS_H
#define SHAPESUTILS_H

#include <QtCore>
#include <QColor>

typedef QList<QPointF> FourPoints;
//Q_DECLARE_METATYPE(FourPoints)

/* shape*/
class Toolshape
{
public:
    QString type; // 图元类型
    // rectangle, oval, effect, arrow, line, pen, text
    // 矩形，椭圆，模糊，箭头，直线，画笔，文本
    FourPoints mainPoints;
    int index = -1;
    int lineWidth = 1; // 线宽
    int colorIndex = 0; // 颜色编号
    bool isBlur = false; // 模糊类型
    int isOval = 0; // 模糊形状 0 椭圆，1 矩形， 2模糊笔
    bool isShiftPressed = false;
    int fontSize = 1; // 字体大小
    int radius = 10;// 模糊强度

    QList<QPointF> points;
    QList<QList<qreal>> portion;
    QPointF arrowRotatePos;
    Toolshape();
    //~Toolshape();

    //friend QDebug &operator<<(QDebug &argument, const Toolshape &obj);
    //friend QDataStream &operator>>(QDataStream &in, Toolshape &obj);
    //Toolshape operator=(Toolshape obj);
    //bool operator==(const Toolshape &other) const;
    //static void registerMetaType();
};

//typedef QList<QPointF> FourPoints;
typedef QList <Toolshape> Toolshapes;
//Q_DECLARE_METATYPE(Toolshape)
//Q_DECLARE_METATYPE(Toolshapes)

#endif // SHAPESUTILS_H
