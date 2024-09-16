// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORLABEL_H
#define COLORLABEL_H
#include "globaldefine.h"

#include <QLabel>
#include <QWidget>

#include <QPaintEvent>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QCursor>

class DRAWLIB_EXPORT ColorLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ColorLabel(QWidget *parent = nullptr);
    ~ColorLabel();

    //h∈(0, 360), s∈(0, 1), v∈(0, 1)
    QColor getColor(qreal h, qreal s, qreal v);
    void setHue(int hue);

    /**
     * @brief pickColor 获取鼠标位置颜色
     * @param pos　位置
     * @param picked　是否获取
     */
    void pickColor(QPoint pos, bool picked = false);

    void setSelectColor(QColor c);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    /**
     * @brief clicked　鼠标点击信号
     */
    void clicked();
    /**
     * @brief pickedColor　获取颜色信号
     * @param color　颜色
     */
    void pickedColor(QColor color);
    /**
     * @brief signalPreViewColor　颜色预览信号
     * @param color　颜色
     */
    void signalPreViewColor(QColor color);

protected:
    void paintEvent(QPaintEvent *) override;

    void resizeEvent(QResizeEvent *event) override;
    /**
     * @brief enterEvent　鼠标进入事件
     * @param e　事件
     */
    void enterEvent(QEvent *e) override;
    /**
     * @brief leaveEvent　鼠标离开事件
     * @param e　事件
     */
    void leaveEvent(QEvent *e) override;
    /**
     * @brief mousePressEvent　鼠标点击事件
     * @param e　鼠标事件
     */
    void mousePressEvent(QMouseEvent *e) override;
    /**
     * @brief mouseMoveEvent　鼠标移动事件
     * @param e　鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *e) override;
    /**
     * @brief mouseReleaseEvent　鼠标弹起事件
     * @param e　鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent *e) override;


    void calImage();

    std::tuple<bool, QPoint> getColorPos(QColor c);

private:
    //calculate the color's rgb value in pos.
    QColor calColorAtPoint(QPointF pos);
    QCursor m_lastCursor;
    int m_hue =  0;
    bool m_workToPick;
    bool m_picking;
    bool m_pressed;
    QColor m_pickedColor;
    QPoint m_clickedPos;
    QPoint m_tipPoint;

    QImage backgroundImage;
    //bool m_cursorEnter;
};

#endif // COLORLABEL_H
