/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LINECHARTNODE_H
#define LINECHARTNODE_H

#include <QColor>
#include <QSGNode>

class QRectF;
class LineChartMaterial;
class LineSegmentNode;

/**
 * @todo write docs
 */
class LineChartNode : public QSGNode
{
public:
    LineChartNode();

    /**
     * Destructor
     */
    ~LineChartNode();

    void setRect(const QRectF &rect, qreal devicePixelRatio);
    void setLineWidth(float width);
    void setLineColor(const QColor &color);
    void setFillColor(const QColor &color);
    void setValues(const QList<QVector2D> &values);
    void updatePoints();

private:
    QRectF m_rect;
    float m_lineWidth = 0.0;
    float m_aspect = 1.0;
    float m_smoothing = 0.1;
    QColor m_lineColor;
    QColor m_fillColor;
    QList<QVector2D> m_values;
};

#endif // LINECHARTNODE_H
