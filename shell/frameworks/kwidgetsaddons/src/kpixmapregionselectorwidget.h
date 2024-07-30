/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPIXMAPREGIONSELECTORWIDGET_H
#define KPIXMAPREGIONSELECTORWIDGET_H

#include <kwidgetsaddons_export.h>

#include <QPixmap>
#include <QWidget>
#include <memory>

class QMenu;

/**
 * @class KPixmapRegionSelectorWidget kpixmapregionselectorwidget.h KPixmapRegionSelectorWidget
 *
 * KPixmapRegionSelectorWidget is a widget that shows a picture and provides the
 * user with a friendly way to select a rectangular subregion of the pixmap.
 *
 * \image html kpixmapregionselectorwidget.png "KPixmapRegionSelectorWidget"
 *
 * @author Antonio Larrosa <larrosa@kde.org>
 */
class KWIDGETSADDONS_EXPORT KPixmapRegionSelectorWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)

public:
    /**
     * This enum provides a rotation direction.
     * @see KPixmapRegionSelectorWidget::rotate()
     */
    enum RotateDirection {
        Rotate90, //!< Rotate 90 degrees to the right.
        Rotate180, //!< Rotate 180 degrees.
        Rotate270 //!< Rotate 90 degrees to the left.
    };

    /**
     * Constructor for a KPixmapRegionSelectorWidget.
     */
    explicit KPixmapRegionSelectorWidget(QWidget *parent = nullptr);

    /**
     * Destructor for a KPixmapRegionSelectorWidget
     */
    ~KPixmapRegionSelectorWidget() override;

    /**
     * Sets the pixmap which will be shown for the user to select a region from.
     * @param pixmap The pixmap.  Must be non-null.
     *
     */
    void setPixmap(const QPixmap &pixmap);

    /**
     * @return the original whole pixmap that we're using in this widget as the
     * pixmap the user is selecting a region from.
     */
    QPixmap pixmap() const;

    /**
     * Sets the selected region to be @p rect (in zoomed pixmap coordinates)
     */
    void setSelectedRegion(const QRect &rect);

    /**
     * Returns the selected region ( in zoomed pixmap coordinates )
     */
    QRect selectedRegion() const;

    /**
     * Returns the selected region ( in unzoomed, original pixmap coordinates )
     */
    QRect unzoomedSelectedRegion() const;

    /**
     * Resets the selection to use the whole image
     */
    void resetSelection();

    /**
     * @returns a QImage object with just the region the user selected from the
     * image
     */
    QImage selectedImage() const;

    /**
     * Sets the aspect ration that the selected subimage should have. The way to
     * select it, is specifying an example valid @p width and @p height.
     * @see setFreeSelectionAspectRatio()
     */
    void setSelectionAspectRatio(int width, int height);

    /**
     * Allows the user to do a selection which has any aspect ratio. This is
     * the default.
     * @see setSelectionAspectRatio()
     */
    void setFreeSelectionAspectRatio();

    /**
     * Sets the maximum size for the widget. If the image is larger than this
     * (either horizontally or vertically), it's scaled to adjust to the maximum
     * size (preserving the aspect ratio)
     */
    void setMaximumWidgetSize(int width, int height);

    /**
     * Rotates the image as specified by the @p direction parameter, also tries
     * to rotate the selected region so that it doesn't change, as long as the
     * forced aspect ratio setting is respected, in other case, the selected region
     * is reset.
     */
    void rotate(RotateDirection direction);

public Q_SLOTS:
    /**
     * Rotates the current image 90º clockwise
     */
    void rotateClockwise();
    /**
     * Rotates the current image 90º counterclockwise
     */
    void rotateCounterclockwise();

Q_SIGNALS:
    void pixmapRotated();

protected:
    /**
     * Creates a QMenu with the menu that appears when clicking with the right button on the label
     */
    virtual QMenu *createPopupMenu();

    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    friend class KPixmapRegionSelectorWidgetPrivate;
    std::unique_ptr<class KPixmapRegionSelectorWidgetPrivate> const d;

    Q_DISABLE_COPY(KPixmapRegionSelectorWidget)
};

#endif
