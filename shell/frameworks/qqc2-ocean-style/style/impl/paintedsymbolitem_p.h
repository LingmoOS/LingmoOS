#ifndef PAINTEDSYMBOLITEM_P_H
#define PAINTEDSYMBOLITEM_P_H

#include "paintedsymbolitem.h"

#include <QFontMetricsF>
#include <QGuiApplication>
#include <QPen>

class PaintedSymbolItemPrivate
{
    Q_DECLARE_PUBLIC(PaintedSymbolItem)
    Q_DISABLE_COPY(PaintedSymbolItemPrivate)
public:
    PaintedSymbolItemPrivate(PaintedSymbolItem *qq)
        : q_ptr(qq)
    {
    }
    PaintedSymbolItem *const q_ptr;

    QColor color;
    qreal penWidth = 1.001; // 1 causes weird distortion
    PaintedSymbolItem::SymbolType symbolType = PaintedSymbolItem::None;

    QPen pen = QPen(color, penWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    QFontMetricsF fontMetrics = QFontMetricsF(QGuiApplication::font());
    qreal targetSymbolSize = 0.0;
    qreal remainder = 0.0;

    // Normalized points. The range is 0 to 1 and the coordinates are scaled to fit the size of the drawing area.
    // clang-format off
    QList<QList<QPointF>> shapePoints = {
        {QPointF(0.0, 0.5), QPointF(1.0/3.0, 5.0/6.0), QPointF(1.0, 1.0/6.0)}, // Checkmark
        {QPointF(1.0/1.5, 0.0), QPointF(0.25, 0.5), QPointF(1.0/1.5, 1.0)}, // LeftArrow
        {QPointF(1.0/3.0, 0.0), QPointF(0.75, 0.5), QPointF(1.0/3.0, 1.0)}, // RightArrow
        {QPointF(0.0, 1.0/1.5), QPointF(0.5, 0.25), QPointF(1.0, 1.0/1.5)}, // UpArrow
        {QPointF(0.0, 1.0/3.0), QPointF(0.5, 0.75), QPointF(1.0, 1.0/3.0)}, // DownArrow
    };
    // clang-format on
};
#endif
