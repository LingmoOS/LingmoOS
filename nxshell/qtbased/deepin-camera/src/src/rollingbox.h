// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROLLINGBOX_H
#define ROLLINGBOX_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPainterPath>

class RollingBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int deviation READ readDeviation WRITE setDeviation)

public:
    explicit RollingBox(QWidget *parent = 0);

    /**
    * @brief setRange 设置范围
    * @param min 最小值
    * @param max 最大值
    */
    void setRange(int min,int max);

    /**
    * @brief setContentList 设置选项列表
    * @param content字符串列表
    */
    void setContentList(const QList<QString> &content);

    /**
    * @brief getCurrentValue 获取当前选项
    */
    int getCurrentValue();

protected:
    /**
    * @brief 鼠标按下事件
    */
    void mousePressEvent(QMouseEvent *) override;

    /**
    * @brief 鼠标移动事件
    */
    void mouseMoveEvent(QMouseEvent *) override;

    /**
    * @brief 鼠标释放事件
    */
    void mouseReleaseEvent(QMouseEvent *) override;

    /**
    * @brief 鼠标滚动事件
    */
    void wheelEvent(QWheelEvent *) override;

    /**
    * @brief focusInEvent 焦点进入事件
    * @param  event 事件参数
    */
    void focusInEvent(QFocusEvent *event) override;

    /**
    * @brief focusOutEvent 焦点离开事件
    * @param  event 事件参数
    */
    void focusOutEvent(QFocusEvent *event) override;

    /**
    * @brief keyReleaseEvent 键盘释放事件
    * @param  event 事件参数
    */
    void keyReleaseEvent(QKeyEvent *event) override;

    /**
    * @brief 绘制事件
    */
    void paintEvent(QPaintEvent *) override;

    /**
    * @brief paintContent 绘制内容
    */
    void paintContent(QPainter &painter,int num,int deviation);

    /**
    * @brief 使选中的内容回到控件中间
    */
    void homing();

    /**
    * @brief readDeviation 获取鼠标移动偏移量，默认为0
    */
    int readDeviation();

    /**
    * @brief setDeviation 设置偏移量
    * @param n 偏移量
    */
    void setDeviation(int n);

signals:
    /**
    * @brief 当前数值改变信号
    */
    void currentValueChanged(int value);

private:
    int                 m_rangeMin;//最小范围
    int                 m_rangeMax;//最大范围
    int                 m_currentIndex;//当前索引
    bool                m_isDragging;//是否鼠标拖动
    bool                m_bFocus;//是否得到焦点
    int                 m_deviation;//偏移量
    int                 m_mouseSrcPos;//鼠标坐标
    int                 m_textSize;    //字符尺寸
    QPropertyAnimation  *m_homingAnimation;//矫正动画
    QTimer              *m_pressResetTimer;//重置鼠标按下状态
    QList<QString>      m_content;//选项列表
};

#endif // ROLLINGBOX_H
