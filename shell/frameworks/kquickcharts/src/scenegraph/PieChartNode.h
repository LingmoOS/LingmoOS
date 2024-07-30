/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PIECHARTNODE_H
#define PIECHARTNODE_H

#include <QColor>
#include <QSGGeometryNode>

class QRectF;
class PieChartMaterial;

/**
 * @todo write docs
 */
class PieChartNode : public QSGGeometryNode
{
public:
    PieChartNode();

    /**
     * Default constructor
     */
    explicit PieChartNode(const QRectF &rect);

    /**
     * Destructor
     */
    ~PieChartNode();

    void setRect(const QRectF &rect);
    void setInnerRadius(qreal radius);
    void setOuterRadius(qreal radius);
    void setSections(const QList<qreal> &sections);
    void setColors(const QList<QColor> &colors);
    void setBackgroundColor(const QColor &color);
    void setFromAngle(qreal angle);
    void setToAngle(qreal angle);
    void setSmoothEnds(bool smooth);

private:
    void updateSegments();

    QRectF m_rect;
    qreal m_innerRadius = 0.0;
    qreal m_outerRadius = 0.0;
    QColor m_backgroundColor;
    qreal m_fromAngle = 0.0;
    qreal m_toAngle = 360.0;
    bool m_smoothEnds = false;

    QList<qreal> m_sections;
    QList<QColor> m_colors;

    QSGGeometry *m_geometry = nullptr;
    PieChartMaterial *m_material = nullptr;
};

#endif // PIECHARTNODE_H
