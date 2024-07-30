/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LINEGRIDNODE_H
#define LINEGRIDNODE_H

#include <QColor>
#include <QSGGeometryNode>

class QSGFlatColorMaterial;

/**
 * @todo write docs
 */
class LineGridNode : public QSGGeometryNode
{
public:
    LineGridNode();
    ~LineGridNode();

    void setVisible(bool visible);
    void setVertical(bool vertical);
    void setRect(const QRectF &rect);
    void setColor(const QColor &color);
    void setSpacing(float spacing);
    void setLineWidth(float lineWidth);

    bool isSubtreeBlocked() const override;

    void update();

private:
    void line(QSGGeometry::Point2D *vertices, quint16 *indices, int &index, qreal fromX, qreal fromY, qreal toX, qreal toY);

    QSGGeometry *m_geometry = nullptr;
    QSGFlatColorMaterial *m_material = nullptr;

    bool m_visible = true;
    bool m_vertical = false;
    QRectF m_rect;
    float m_spacing = 1.0;
    float m_lineWidth = 1.0;
};

#endif // LINEGRIDNODE_H
