// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRIDLINEWIDGET_H
#define GRIDLINEWIDGET_H

#include "datamanager.h"

#include <QWidget>
#include <QGraphicsItem>

//网格线图元类，用于在QGraphicsScene画布中显示网格线，与网格线部件互斥显示
class GridLineItem : public QGraphicsItem {
public:
    explicit GridLineItem(QGraphicsItem* parent = nullptr);
    // 设置网格线类型
    void setGridType(GridType type);

    virtual QRectF boundingRect() const override;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    GridType m_gridType; // 当前显示的网格线类型
    QRectF m_boundingRect;
};

class videowidget;
//网格线部件类，用于在OpenGL窗口中显示网格线，与网格线图元互斥显示
class GridLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GridLineWidget(QWidget *parent = nullptr);
    // 设置网格线类型
    void setGridType(GridType type);

protected:
    virtual void paintEvent(QPaintEvent * e) override;

private:
    GridType m_gridType; // 当前显示的网格线类型
    videowidget *m_videowidget;
};

#endif // GRIDLINEWIDGET_H
