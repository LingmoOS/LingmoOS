/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BARCHARTNODE_H
#define BARCHARTNODE_H

#include <QColor>
#include <QSGGeometryNode>

struct Bar {
    float x;
    float width;
    float value;
    QColor color;
};

class BarChartNode : public QSGNode
{
public:
    BarChartNode();

    void setRect(const QRectF &rect);
    void setBars(const QList<Bar> &bars);
    void setRadius(qreal radius);
    void setBackgroundColor(const QColor &color);
    void update();

private:
    QRectF m_rect;
    QList<Bar> m_bars;
    qreal m_radius = 0.0;
    QColor m_backgroundColor = Qt::transparent;
};

#endif // BARCHARTNODE_H
