/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QIMAGEITEM_H
#define QIMAGEITEM_H

#include <QImage>
#include <QQuickPaintedItem>

class QImageItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged RESET resetImage)
    Q_PROPERTY(bool smooth READ smooth WRITE setSmooth)
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
        Pad, /**< the image is not transformed @since 5.96 **/
    };
    Q_ENUM(FillMode)

    explicit QImageItem(QQuickItem *parent = nullptr);
    ~QImageItem() override;

    static void initDamage();
    void setImage(const QImage &image);
    QImage image() const;
    void resetImage();

    void setSmooth(const bool smooth);
    bool smooth() const;

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
    void imageChanged();
    void nullChanged();
    void paintedWidthChanged();
    void paintedHeightChanged();

protected:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#else
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#endif

private:
    static QImage s_damage;
private:
    QImage m_image;
    bool m_smooth;
    FillMode m_fillMode;
    QRect m_paintedRect;

private Q_SLOTS:
    void updatePaintedRect();
};

#endif
