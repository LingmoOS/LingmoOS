/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QPIXMAPITEM_H
#define QPIXMAPITEM_H

#include <QPixmap>
#include <QQuickPaintedItem>

class QPixmapItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap NOTIFY pixmapChanged RESET resetPixmap)
    Q_PROPERTY(int nativeWidth READ nativeWidth NOTIFY nativeWidthChanged)
    Q_PROPERTY(int nativeHeight READ nativeHeight NOTIFY nativeHeightChanged)
    Q_PROPERTY(int paintedWidth READ paintedWidth NOTIFY paintedWidthChanged)
    Q_PROPERTY(int paintedHeight READ paintedHeight NOTIFY paintedHeightChanged)
    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    Q_PROPERTY(bool null READ isNull NOTIFY nullChanged)

public:
    enum FillMode {
        Stretch, // the image is scaled to fit
        PreserveAspectFit, // the image is scaled uniformly to fit without cropping
        PreserveAspectCrop, // the image is scaled uniformly to fill, cropping if necessary
        Tile, // the image is duplicated horizontally and vertically
        TileVertically, // the image is stretched horizontally and tiled vertically
        TileHorizontally, // the image is stretched vertically and tiled horizontally
    };
    Q_ENUM(FillMode)

    explicit QPixmapItem(QQuickItem *parent = nullptr);
    ~QPixmapItem() override;

    void setPixmap(const QPixmap &pixmap);
    QPixmap pixmap() const;
    void resetPixmap();

    int nativeWidth() const;
    int nativeHeight() const;

    int paintedWidth() const;
    int paintedHeight() const;

    FillMode fillMode() const;
    void setFillMode(FillMode mode);

    void paint(QPainter *painter) override;

    bool isNull() const;

Q_SIGNALS:
    void nativeWidthChanged();
    void nativeHeightChanged();
    void fillModeChanged();
    void pixmapChanged();
    void nullChanged();
    void paintedWidthChanged();
    void paintedHeightChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    QPixmap m_pixmap;
    FillMode m_fillMode;
    QRect m_paintedRect;

private Q_SLOTS:
    void updatePaintedRect();
};

#endif
