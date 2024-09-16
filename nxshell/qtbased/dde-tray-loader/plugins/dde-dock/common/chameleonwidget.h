// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CHAMELEONWIDGET_H
#define CHAMELEONWIDGET_H

#include "mousestatewidget.h"

#include <QWidget>

/**
 * @brief hover 和 press 状态的时候叠加一层透明色
 */
class ChameleonWidget : public MouseStateWidget
{
    Q_OBJECT
public:
    ChameleonWidget(QWidget *parent = nullptr);
    ~ChameleonWidget();

    /**
     * @brief 设置是否在hover 和 press 状态的时候变色
     */
    void setBeChameleon(bool beChameleon) { m_beChameleon = beChameleon; }
    void setDraggingState(bool dragging);
    void setActive(bool active);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawBorder(QPainter *p, const QRectF &rect, double radius);

private:
    bool m_beChameleon;
    bool m_dragging;
    bool m_active;
};

#endif
