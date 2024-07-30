/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LINESEGMENTNODE_H
#define LINESEGMENTNODE_H

#include <QColor>
#include <QSGGeometryNode>
#include <QVector2D>

class QRectF;
class LineChartMaterial;

/**
 * @todo write docs
 */
class LineSegmentNode : public QSGGeometryNode
{
public:
    LineSegmentNode();

    /**
     * Default constructor
     */
    explicit LineSegmentNode(const QRectF &rect);

    /**
     * Destructor
     */
    ~LineSegmentNode();

    void setRect(const QRectF &rect);
    void setAspect(float xAspect, float yAspect);
    void setSmoothing(float smoothing);
    void setLineWidth(float width);
    void setLineColor(const QColor &color);
    void setFillColor(const QColor &color);
    void setValues(const QList<QVector2D> &values);
    void setFarLeft(const QVector2D &value);
    void setFarRight(const QVector2D &value);

    void update();

private:
    QRectF m_rect;
    float m_lineWidth = 0.0;
    float m_xAspect = 1.0;
    float m_yAspect = 1.0;
    float m_smoothing = 0.1;
    QVector2D m_farLeft;
    QVector2D m_farRight;
    QList<QVector2D> m_values;
    QSGGeometry *m_geometry = nullptr;
    LineChartMaterial *m_material = nullptr;
    QColor m_lineColor;
    QColor m_fillColor;
};

#endif // LINESEGMENTNODE_H
