/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qimageitem.h"

#include <QPainter>

QImageItem::QImageItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_fillMode(QImageItem::Stretch)
{
    setFlag(ItemHasContents, true);
}

QImageItem::~QImageItem()
{
}

void QImageItem::setImage(const QImage &image)
{
    bool oldImageNull = m_image.isNull();
    m_image = image;
    updatePaintedRect();
    update();
    Q_EMIT nativeWidthChanged();
    Q_EMIT nativeHeightChanged();
    Q_EMIT imageChanged();
    if (oldImageNull != m_image.isNull()) {
        Q_EMIT nullChanged();
    }
}

QImage QImageItem::image() const
{
    return m_image;
}

void QImageItem::resetImage()
{
    setImage(QImage());
}

int QImageItem::nativeWidth() const
{
    return m_image.size().width() / m_image.devicePixelRatio();
}

int QImageItem::nativeHeight() const
{
    return m_image.size().height() / m_image.devicePixelRatio();
}

QImageItem::FillMode QImageItem::fillMode() const
{
    return m_fillMode;
}

void QImageItem::setFillMode(QImageItem::FillMode mode)
{
    if (mode == m_fillMode) {
        return;
    }

    m_fillMode = mode;
    updatePaintedRect();
    update();
    Q_EMIT fillModeChanged();
}

void QImageItem::paint(QPainter *painter)
{
    if (m_image.isNull()) {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, smooth());
    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth());

    if (m_fillMode == TileVertically) {
        painter->scale(width() / (qreal)m_image.width(), 1);
    }

    if (m_fillMode == TileHorizontally) {
        painter->scale(1, height() / (qreal)m_image.height());
    }

    if (m_fillMode == Pad) {
        QRect centeredRect = m_paintedRect;
        centeredRect.moveCenter(m_image.rect().center());
        painter->drawImage(m_paintedRect, m_image, centeredRect);
    } else if (m_fillMode >= Tile) {
        painter->drawTiledPixmap(m_paintedRect, QPixmap::fromImage(m_image));
    } else {
        painter->drawImage(m_paintedRect, m_image, m_image.rect());
    }

    painter->restore();
}

bool QImageItem::isNull() const
{
    return m_image.isNull();
}

int QImageItem::paintedWidth() const
{
    if (m_image.isNull()) {
        return 0;
    }

    return m_paintedRect.width();
}

int QImageItem::paintedHeight() const
{
    if (m_image.isNull()) {
        return 0;
    }

    return m_paintedRect.height();
}

void QImageItem::updatePaintedRect()
{
    if (m_image.isNull()) {
        return;
    }

    QRectF sourceRect = m_paintedRect;

    QRectF destRect;

    switch (m_fillMode) {
    case PreserveAspectFit: {
        QSizeF scaled = m_image.size();

        scaled.scale(boundingRect().size(), Qt::KeepAspectRatio);
        destRect = QRectF(QPoint(0, 0), scaled);
        destRect.moveCenter(boundingRect().center().toPoint());
        break;
    }
    case PreserveAspectCrop: {
        QSizeF scaled = m_image.size();

        scaled.scale(boundingRect().size(), Qt::KeepAspectRatioByExpanding);
        destRect = QRectF(QPoint(0, 0), scaled);
        destRect.moveCenter(boundingRect().center().toPoint());
        break;
    }
    case TileVertically: {
        destRect = boundingRect().toRect();
        destRect.setWidth(destRect.width() / (width() / (qreal)m_image.width()));
        break;
    }
    case TileHorizontally: {
        destRect = boundingRect().toRect();
        destRect.setHeight(destRect.height() / (height() / (qreal)m_image.height()));
        break;
    }
    case Stretch:
    case Tile:
    case Pad:
    default:
        destRect = boundingRect().toRect();
    }

    if (destRect != sourceRect) {
        m_paintedRect = destRect.toRect();
        Q_EMIT paintedHeightChanged();
        Q_EMIT paintedWidthChanged();
    }
}
void QImageItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    updatePaintedRect();
}

#include "moc_qimageitem.cpp"
