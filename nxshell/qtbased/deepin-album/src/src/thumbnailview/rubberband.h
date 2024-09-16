// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RUBBERBAND_H
#define RUBBERBAND_H

#include <QQuickPaintedItem>

class RubberBand : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit RubberBand(QQuickItem *parent = nullptr);
    ~RubberBand() override;

    void paint(QPainter *painter) override;

    Q_INVOKABLE bool intersects(const QRectF &rect) const;

protected:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#else
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#endif

private:
    QRectF m_geometry;
};

#endif
