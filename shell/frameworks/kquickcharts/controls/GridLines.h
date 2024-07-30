/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GRIDLINES_H
#define GRIDLINES_H

#include <memory>

#include <QQuickItem>

class GridLines;
class LineGridNode;
class XYChart;

class LinePropertiesGroup : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Grouped Property")

public:
    explicit LinePropertiesGroup(GridLines *parent);

    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY propertiesChanged)
    bool visible() const;
    void setVisible(bool newVisible);

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY propertiesChanged)
    QColor color() const;
    void setColor(const QColor &newColor);

    Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth NOTIFY propertiesChanged)
    float lineWidth() const;
    void setLineWidth(float newLineWidth);

    Q_PROPERTY(int frequency READ frequency WRITE setFrequency NOTIFY propertiesChanged)
    int frequency() const;
    void setFrequency(int newFrequency);

    Q_PROPERTY(int count READ count WRITE setCount NOTIFY propertiesChanged)
    int count() const;
    void setCount(int newCount);

    Q_SIGNAL void propertiesChanged();

private:
    GridLines *m_parent = nullptr;
    bool m_visible = true;
    QColor m_color = Qt::black;
    float m_lineWidth = 1.0;
    int m_frequency = 2;
    int m_count = -1;
};

/**
 * An item that renders a set of lines to make a grid for a chart.
 */
class GridLines : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum class Direction { Horizontal, Vertical };
    Q_ENUM(Direction)
    /**
     * Default constructor
     */
    explicit GridLines(QQuickItem *parent = nullptr);

    Q_PROPERTY(GridLines::Direction direction READ direction WRITE setDirection NOTIFY directionChanged)
    Direction direction() const;
    void setDirection(GridLines::Direction newDirection);
    Q_SIGNAL void directionChanged();

    Q_PROPERTY(XYChart *chart READ chart WRITE setChart NOTIFY chartChanged)
    XYChart *chart() const;
    void setChart(XYChart *newChart);
    Q_SIGNAL void chartChanged();

    Q_PROPERTY(float spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)
    float spacing() const;
    void setSpacing(float newSpacing);
    Q_SIGNAL void spacingChanged();

    Q_PROPERTY(LinePropertiesGroup *major READ majorGroup CONSTANT)
    LinePropertiesGroup *majorGroup() const;

    Q_PROPERTY(LinePropertiesGroup *minor READ minorGroup CONSTANT)
    LinePropertiesGroup *minorGroup() const;

private:
    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *) override;
    void updateLines(LineGridNode *node, LinePropertiesGroup *properties);

    GridLines::Direction m_direction = Direction::Horizontal;
    XYChart *m_chart = nullptr;
    float m_spacing = 10.0;

    std::unique_ptr<LinePropertiesGroup> m_major;
    std::unique_ptr<LinePropertiesGroup> m_minor;
};

#endif // GRIDLINES_H
