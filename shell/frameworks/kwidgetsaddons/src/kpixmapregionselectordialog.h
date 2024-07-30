/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPIXMAPREGIONSELECTORDIALOG_H
#define KPIXMAPREGIONSELECTORDIALOG_H

#include <QDialog>
#include <memory>

#include <kwidgetsaddons_export.h>

class KPixmapRegionSelectorWidget;

class QImage;

/**
 * @class KPixmapRegionSelectorDialog kpixmapregionselectordialog.h KPixmapRegionSelectorDialog
 *
 * A dialog that uses a KPixmapRegionSelectorWidget to allow the user
 * to select a region of an image. If you want to use special features
 * like forcing the selected area to have a fixed aspect ratio, you can use
 * pixmapRegionSelectorWidget() to get the pointer to the
 * KPixmapRegionSelectorWidget object and set the desired options there.
 *
 * There are some convenience methods that allow to easily show a dialog
 * for the user to select a region of an image, and just care about the selected
 * image.
 *
 * \image html kpixmapregionselectordialog.png "KPixmapRegionSelectorDialog"
 *
 * @author Antonio Larrosa <larrosa@kde.org>
 */
class KWIDGETSADDONS_EXPORT KPixmapRegionSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * The constructor of an empty KPixmapRegionSelectorDialog, you have to call
     * later the setPixmap method of the KPixmapRegionSelectorWidget widget of
     * the new object.
     */
    explicit KPixmapRegionSelectorDialog(QWidget *parent = nullptr);

    /**
     * The destructor of the dialog
     */
    ~KPixmapRegionSelectorDialog() override;

    /**
     * @returns the KPixmapRegionSelectorWidget widget so that additional
     * parameters can be set by using it.
     */
    KPixmapRegionSelectorWidget *pixmapRegionSelectorWidget() const;

    /**
     * Creates a modal dialog, lets the user to select a region of the @p pixmap
     * and returns when the dialog is closed.
     *
     * @returns the selected rectangle, or an invalid rectangle if the user
     * pressed the Cancel button.
     */
    static QRect getSelectedRegion(const QPixmap &pixmap, QWidget *parent = nullptr);

    /**
     * Creates a modal dialog, lets the user to select a region of the @p pixmap
     * with the same aspect ratio than @p aspectRatioWidth x @p aspectRatioHeight
     * and returns when the dialog is closed.
     *
     * @returns the selected rectangle, or an invalid rectangle if the user
     * pressed the Cancel button.
     */
    static QRect getSelectedRegion(const QPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, QWidget *parent = nullptr);

    /**
     * Creates a modal dialog, lets the user to select a region of the @p pixmap
     * and returns when the dialog is closed.
     *
     * @returns the selected image, or an invalid image if the user
     * pressed the Cancel button.
     */
    static QImage getSelectedImage(const QPixmap &pixmap, QWidget *parent = nullptr);

    /**
     * Creates a modal dialog, lets the user to select a region of the @p pixmap
     * with the same aspect ratio than @p aspectRatioWidth x @p aspectRatioHeight
     * and returns when the dialog is closed.
     *
     * @returns the selected image, or an invalid image if the user
     * pressed the Cancel button.
     */
    static QImage getSelectedImage(const QPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, QWidget *parent = nullptr);

    /**
     * @since 4.4.3
     * Adjusts the size of the KPixmapRegionSelectorWidget to not overflow the screen size
     */
    void adjustRegionSelectorWidgetSizeToFitScreen();

private:
    std::unique_ptr<class KPixmapRegionSelectorDialogPrivate> const d;

    Q_DISABLE_COPY(KPixmapRegionSelectorDialog)
};

#endif
