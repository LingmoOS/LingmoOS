// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHART_VIEW_WIDGET_H
#define CHART_VIEW_WIDGET_H

#include <QWidget>
#include <QVariant>
#include <QPainterPath>

class ChartViewWidget : public QWidget
{
    Q_OBJECT

public:
    enum ChartViewTypes {
        MEM_CHART,      //内存
        NET_CHART,      //网络
        BLOCK_CHART     //磁盘
    };
    explicit ChartViewWidget(ChartViewWidget::ChartViewTypes types, QWidget *parent = nullptr);


public:
    void setData1Color(const QColor &color);
    void addData1(const QVariant &data);

    void setData2Color(const QColor &color);
    void addData2(const QVariant &data);

    void setSpeedAxis(bool speed);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);

private slots:
    void changeFont(const QFont &font);
    void changeTheme();

private:
    void drawBackPixmap();
    void drawData1(QPainter *painter);
    void drawData2(QPainter *painter);
    void drawData3(QPainter *painter);
    void drawAxisText(QPainter *painter);

    void setAxisTitle(const QString &text);
    void getPainterPathByData(const QList<QVariant> &listData, QPainterPath &path,  QVariant maxYvalue);

private:
    int gridSize = 10;

    QString m_axisTitle = {"100%"};

    QFont m_textfont;
    QPixmap m_backPixmap = QPixmap();

    QRect m_chartRect;
    QColor m_data1Color = {"#00C5C0"};
    QColor m_data2Color = {"#FEDF19"};

    bool  m_speedAxis = false;

    QVariant m_maxData  = 1;
    QVariant m_maxData1 = 1;
    QVariant m_maxData2 = 1;

    QList<QVariant> m_listData1;
    QList<QVariant> m_listData2;

    ChartViewTypes m_viewType = ChartViewTypes::MEM_CHART;  // 图表界面类型
};

#endif // CHART_VIEW_WIDGET_H
