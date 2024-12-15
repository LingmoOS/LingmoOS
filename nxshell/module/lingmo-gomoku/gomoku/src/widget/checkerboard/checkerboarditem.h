// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKERBOARDITEM_H
#define CHECKERBOARDITEM_H

#include <DHiDPIHelper>

#include <QGraphicsItem>

DWIDGET_USE_NAMESPACE

class CheckerboardItem : public QGraphicsItem
{
public:
    explicit CheckerboardItem(QGraphicsItem *parent = nullptr);
    ~CheckerboardItem() override;

public:
    QRectF boundingRect() const override;

public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QPixmap  checkerboardPixmap; //背景图片
    const qreal CheckerboardPosWidth = 0.02; //棋盘位置占整个scene宽度比例
    const qreal CheckerboardPosHeight = 0.007; //棋盘位置占整个scene高度比例
};

#endif // CHECKERBOARDITEM_H
