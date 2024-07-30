/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qpixmapitem.h"

#include <QPainter>

QPixmapItem::QPixmapItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_fillMode(QPixmapItem::Stretch)
{
    setFlag(ItemHasContents, true);
}

QPixmapItem::~QPixmapItem()
{
}

void QPixmapItem::setPixmap(const QPixmap &pixmap)
{
    bool oldPixmapNull = m_pixmap.isNull();
    m_pixmap = pixmap;
    updatePaintedRect();
    update();
    Q_EMIT nativeWidthChanged();
    Q_EMIT nativeHeightChanged();
    Q_EMIT pixmapChanged();
    if (oldPixmapNull != m_pixmap.isNull()) {
        Q_EMIT nullChanged();
    }
}

QPixmap QPixmapItem::pixmap() const
{
    return m_pixmap;
}

void QPixmapItem::resetPixmap()
{
    setPixmap(QPixmap());
}

int QPixmapItem::nativeWidth() const
{
    return m_pixmap.size().width() / m_pixmap.devicePixelRatio();
}

int QPixmapItem::nativeHeight() const
{
    return m_pixmap.size().height() / m_pixmap.devicePixelRatio();
}

QPixmapItem::FillMode QPixmapItem::fillMode() const
{
    return m_fillMode;
}

void QPixmapItem::setFillMode(QPixmapItem::FillMode mode)
{
    if (mode == m_fillMode) {
        return;
    }

    m_fillMode = mode;
    updatePaintedRect();
    update();
    Q_EMIT fillModeChanged();
}

void QPixmapItem::paint(QPainter *painter)
{
    if (m_pixmap.isNull()) {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, smooth());
    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth());

    if (m_fillMode == TileVertically) {
        painter->scale(width() / (qreal)m_pixmap.width(), 1);
    }

    if (m_fillMode == TileHorizontally) {
        painter->scale(1, height() / (qreal)m_pixmap.height());
    }

    if (m_fillMode >= Tile) {
        painter->drawTiledPixmap(m_paintedRect, m_pixmap);
    } else {
        painter->drawPixmap(m_paintedRect, m_pixmap, m_pixmap.rect());
    }

    painter->restore();
}

bool QPixmapItem::isNull() const
{
    return m_pixmap.isNull();
}

int QPixmapItem::paintedWidth() const
{
    if (m_pixmap.isNull()) {
        return 0;
    }

    return m_paintedRect.width();
}

int QPixmapItem::paintedHeight() const
{
    if (m_pixmap.isNull()) {
        return 0;
    }

    return m_paintedRect.height();
}

void QPixmapItem::updatePaintedRect()
{
    if (m_pixmap.isNull()) {
        return;
    }

    QRectF sourceRect = m_paintedRect;

    QRectF destRect;

    switch (m_fillMode) {
    case PreserveAspectFit: {
        QSizeF scaled = m_pixmap.size();

        scaled.scale(boundingRect().size(), Qt::KeepAspectRatio);
        destRect = QRectF(QPoint(0, 0), scaled);
        destRect.moveCenter(boundingRect().center().toPoint());

        break;
    }
    case PreserveAspectCrop: {
        QSizeF scaled = m_pixmap.size();

        scaled.scale(boundingRect().size(), Qt::KeepAspectRatioByExpanding);
        destRect = QRectF(QPoint(0, 0), scaled);
        destRect.moveCenter(boundingRect().center().toPoint());
        break;
    }
    case TileVertically: {
        destRect = boundingRect().toRect();
        destRect.setWidth(destRect.width() / (width() / (qreal)m_pixmap.width()));
        break;
    }
    case TileHorizontally: {
        destRect = boundingRect().toRect();
        destRect.setHeight(destRect.height() / (height() / (qreal)m_pixmap.height()));
        break;
    }
    case Stretch:
    case Tile:
    default:
        destRect = boundingRect().toRect();
    }

    if (destRect != sourceRect) {
        m_paintedRect = destRect.toRect();
        Q_EMIT paintedHeightChanged();
        Q_EMIT paintedWidthChanged();
    }
}

void QPixmapItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    updatePaintedRect();
}

#include "moc_qpixmapitem.cpp"
