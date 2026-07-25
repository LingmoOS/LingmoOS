#ifndef LINGMORECTANGLE_H
#define LINGMORECTANGLE_H

#include <QQmlEngine>
#include <QPainter>
#include <QPainterPath>
#include <QQuickPaintedItem>

#include "stdafx.h"

class LingmoRectangle : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY_AUTO(QColor, color)
    Q_PROPERTY_AUTO(QList<int>, radius)
    QML_NAMED_ELEMENT(LingmoRectangle)

public:
    explicit LingmoRectangle(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;
};

#endif // LINGMORECTANGLE_H
