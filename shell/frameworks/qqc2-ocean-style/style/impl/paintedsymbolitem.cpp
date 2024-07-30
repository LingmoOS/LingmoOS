/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "paintedsymbolitem.h"
#include "paintedsymbolitem_p.h"
#include <QGuiApplication>
#include <QPainter>

PaintedSymbolItem::PaintedSymbolItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , d_ptr(new PaintedSymbolItemPrivate(this))
{
    Q_D(PaintedSymbolItem);
    connect(qGuiApp, &QGuiApplication::fontChanged, this, [this, d]() {
        d->fontMetrics = QFontMetricsF(QGuiApplication::font());
        update();
    });
    d->targetSymbolSize = d->fontMetrics.height() + std::fmod(d->fontMetrics.height(), 2.0);
    d->targetSymbolSize -= std::fmod(d->targetSymbolSize, 6.0);
    d->targetSymbolSize -= d->targetSymbolSize / 3.0;
    setImplicitSize(d->targetSymbolSize, d->fontMetrics.height());
    setBaselineOffset(d->fontMetrics.ascent());
}

PaintedSymbolItem::~PaintedSymbolItem() noexcept
{
}

void PaintedSymbolItem::paint(QPainter *painter)
{
    Q_D(PaintedSymbolItem);
    // Do I need to save() and restore() if I change the painter in the switch blocks?
    switch (d->symbolType) {
    case SymbolType::Checkmark: {
        setPenWidth(2);
        qreal penOffset = d->penWidth / 2;

        // Prevent the sides from being cut off. Remember to add extra width and height externally.
        qreal width = this->width() - d->penWidth * 2;
        qreal height = this->height() - d->penWidth * 2;

        // No point in trying to draw if it's too small
        if (std::min(width, height) <= 0) {
            return;
        }

        painter->translate(d->penWidth, d->penWidth);

        painter->setBrush(Qt::NoBrush);
        painter->setPen(d->pen);
        painter->setRenderHint(QPainter::Antialiasing);

        QList<QPointF> points = {
            QPointF(0 + penOffset, height / 2.0 + penOffset),
            QPointF(width / 3.0, height / 1.2), // height * (5/6)
            QPointF(width - penOffset, height / 6.0 + penOffset),
        };
        painter->drawPolyline(points);

    } break;
    case SymbolType::LeftArrow: {
        setPenWidth(1);
        qreal penOffset = d->penWidth / 2;

        // Prevent the sides from being cut off. Remember to add extra width and height externally.
        qreal width = this->width() - d->penWidth * 2;
        qreal height = this->height() - d->penWidth * 2;

        // No point in trying to draw if it's too small
        if (std::min(width, height) <= 0) {
            return;
        }

        painter->translate(d->penWidth, d->penWidth);

        painter->setBrush(Qt::NoBrush);
        painter->setPen(d->pen);
        painter->setRenderHint(QPainter::Antialiasing);

        QList<QPointF> points = {
            QPointF(width / 1.5 - penOffset, 0 + penOffset),
            QPointF(width / 4.0, height / 2),
            QPointF(width / 1.5 - penOffset, height - penOffset),
        };
        painter->drawPolyline(points);

    } break;
    case SymbolType::RightArrow: {
        setPenWidth(1);
        // qreal penOffset = d->penWidth / 2;

        // Prevent the sides from being cut off. Remember to add extra width and height externally.
        qreal width = this->width() - d->penWidth * 2;
        qreal height = this->height() - d->penWidth * 2;

        // No point in trying to draw if it's too small
        if (std::min(width, height) <= 0) {
            return;
        }

        painter->translate(d->penWidth, d->penWidth);

        painter->setBrush(Qt::NoBrush);
        painter->setPen(d->pen);
        painter->setRenderHint(QPainter::Antialiasing);

        QList<QPointF> points = {
            QPointF(width / 3 /* + penOffset*/, 0 /* + penOffset*/),
            QPointF(width * 0.75, height / 2),
            QPointF(width / 3 /* + penOffset*/, height /* - penOffset*/),
        };
        painter->drawPolyline(points);

    } break;
    case SymbolType::UpArrow: {
        setPenWidth(1);
        qreal penOffset = d->penWidth / 2;

        // Prevent the sides from being cut off. Remember to add extra width and height externally.
        qreal width = this->width() - d->penWidth * 2;
        qreal height = this->height() - d->penWidth * 2;

        // No point in trying to draw if it's too small
        if (std::min(width, height) <= 0) {
            return;
        }

        painter->translate(d->penWidth, d->penWidth + penOffset);

        painter->setBrush(Qt::NoBrush);
        painter->setPen(d->pen);
        painter->setRenderHint(QPainter::Antialiasing);

        QList<QPointF> points = {
            QPointF(0 - penOffset, height * 0.75),
            QPointF(width / 2.0, height / 4 - penOffset),
            QPointF(width + penOffset, height * 0.75),
        };
        painter->drawPolyline(points);

    } break;
    case SymbolType::DownArrow: {
        setPenWidth(1);
        qreal penOffset = d->penWidth / 2;

        // Prevent the sides from being cut off. Remember to add extra width and height externally.
        qreal width = this->width() - d->penWidth * 2;
        qreal height = this->height() - d->penWidth * 2;

        // No point in trying to draw if it's too small
        if (std::min(width, height) <= 0) {
            return;
        }

        painter->translate(d->penWidth, d->penWidth + penOffset);

        painter->setBrush(Qt::NoBrush);
        painter->setPen(d->pen);
        painter->setRenderHint(QPainter::Antialiasing);

        QList<QPointF> points = {
            QPointF(0 - penOffset, height / 4),
            QPointF(width / 2.0, height * 0.75 + penOffset),
            QPointF(width + penOffset, height / 4),
        };
        painter->drawPolyline(points);

    } break;
    default:
        break;
    }
}

// color
QColor PaintedSymbolItem::color() const
{
    Q_D(const PaintedSymbolItem);
    return d->color;
}

void PaintedSymbolItem::setColor(const QColor &color)
{
    Q_D(PaintedSymbolItem);
    if (color == d->color) {
        return;
    }

    d->color = color;
    d->pen.setColor(d->color);
    update();
    Q_EMIT colorChanged();
}

// penWidth
qreal PaintedSymbolItem::penWidth() const
{
    Q_D(const PaintedSymbolItem);
    return d->penWidth;
}

void PaintedSymbolItem::setPenWidth(const qreal penWidth)
{
    Q_D(PaintedSymbolItem);
    if (penWidth == d->penWidth || (d->penWidth == 1.001 && penWidth == 1)) {
        return;
    }

    d->penWidth = penWidth == 1 ? 1.001 : penWidth;
    d->pen.setWidthF(d->penWidth);
    Q_EMIT penWidthChanged();
}

// symbolType
PaintedSymbolItem::SymbolType PaintedSymbolItem::symbolType() const
{
    Q_D(const PaintedSymbolItem);
    return d->symbolType;
}

void PaintedSymbolItem::setSymbolType(const SymbolType symbolType)
{
    Q_D(PaintedSymbolItem);
    if (symbolType == d->symbolType) {
        return;
    }

    d->symbolType = symbolType;
    update();
    Q_EMIT symbolTypeChanged();
}

#include "moc_paintedsymbolitem.cpp"
