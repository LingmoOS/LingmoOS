// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMCONTROL_ROUNDCOLORWIDGET_H
#define CUSTOMCONTROL_ROUNDCOLORWIDGET_H

#include <DWidget>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

/**
 * @brief The RoundColorWidget class
 * 颜色选项框控件
 */
class RoundColorWidget : public DWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(RoundColorWidget)

public:
    explicit RoundColorWidget(const QColor &color, QWidget *parent = nullptr);

    /**
     * @brief setSelected
     * 设置是否选中
     * @param selected
     */
    void setSelected(bool selected);

    /**
     * @brief setAllClickNotify
     * 点击是否发送点击信号
     * @param bnotify
     */
    inline void setAllClickNotify(bool bnotify) {m_allnotify = bnotify;}

Q_SIGNALS:
    /**
     * @brief clicked
     * 点击信号
     */
    void clicked();

protected:
    /**
     * @brief mousePressEvent
     * 鼠标点击事件
     * @param event
     */
    void mousePressEvent(QMouseEvent *event);

    /**
     * @brief paintEvent
     * 绘制事件
     * @param event
     */
    void paintEvent(QPaintEvent *event);

private:
    bool m_isSelected = false;
    QColor m_color;
    bool m_allnotify = false;
};

#endif // CUSTOMCONTROL_ROUNDCOLORWIDGET_H
