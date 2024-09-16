// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkwidget_global.h>

#include <QFrame>
#include <QPen>
#include <QBrush>
#include <QPainter>

class BackgroundFrame : public QFrame
{
    Q_OBJECT
public:
    explicit BackgroundFrame(QWidget *parent = nullptr, int cornerRadius = 8);

    void setRadius(qreal xRadius, qreal yRadius);
    void setPenColor(const QColor &color) {
        m_pen.setColor(color);
    }
    void setPenStyle(const Qt::PenStyle style) {
        m_pen.setStyle(style);
    }
    void setBrush(QBrush brush) {
        m_brush = brush;
    }
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

private:
    QPainter    *m_painter;
    QPen        m_pen;
    QBrush      m_brush;
    qreal       m_xRadius;
    qreal       m_yRadius;
};
