// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef PARTCHARTSHOWING_H
#define PARTCHARTSHOWING_H

#include <DToolTip>
#include <DWidget>
#include <DApplicationHelper>
#include <DPalette>

#include <QWidget>
#include <QRect>
#include <QPainterPath>
#include <QPainter>
#include <QBrush>
#include <QPointF>
#include <QLine>
#include <QMouseEvent>
#include <QTimer>

DWIDGET_USE_NAMESPACE

/**
 * @class PartChartShowing
 * @brief 分区占比图表显示类
 */

class PartChartShowing : public QWidget
{
    Q_OBJECT
public:
    explicit PartChartShowing(QWidget *parent = nullptr);
    //    PartChartShowing(int used, int trueused, int noused, QWidget *parent = nullptr);

    /**
     * @brief 设置新建分区信息
     * @param totals 空闲空间总大小
     * @param sizeInfo 每个分区大小列表
     */
    void transInfos(const double &totals, const QVector<double> sizeInfo);

    /**
     * @brief 设置新建分区信息
     * @param mflag 分区标志
     * @param value 分区大小值
     */
    void transFlag(int flag, int value = 0);

signals:
    void sendFlag(int flag, int num, int posX);
    void sendMoveFlag(int moveflag, int num, int posx);
    void judgeLastPartition();

private slots:

    /**
     * @brief 定时器超时信号响应的槽函数
     */
    void showTipTimerSlot();

protected:

    /**
     * @brief 绘制事件
     */
    virtual void paintEvent(QPaintEvent *event) override;

    /**
     * @brief 添加绘画
     * @param painter 绘画
     */
    void addPaint(QPainter *painter);

    /**
     * @brief 鼠标点击事件
     */
    virtual void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件
     */
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    int m_flag = 1;
    double m_total = 0.00;
    QVector<double> m_partSize;
    QVector<QColor> m_baseColor;
    QVector<QPainterPath> m_allPath;
    QTimer *m_showTipTimer;
    int m_number = -1;
    int m_number2 = -1;
    double m_sums = 0.00;
//    int i = 0;
//    int count = 0;
    double m_sumValue = 0.0000001;
    int m_hover = 0;
    int m_posX = 0;
    int m_showTipFlag = 0;
    bool m_isShowFlag = false;
//    int m_mnum = 0;
};

#endif // PARTCHARTSHOWING_H
