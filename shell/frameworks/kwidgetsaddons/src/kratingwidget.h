/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006-2007 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRATINGWIDGET_H
#define KRATINGWIDGET_H

#include <QFrame>
#include <memory>

#include <kwidgetsaddons_export.h>

/**
 * \class KRatingWidget kratingwidget.h KRatingWidget
 *
 * \brief Displays a rating value as a row of pixmaps.
 *
 * The KRatingWidget displays a range of stars or other arbitrary
 * pixmaps and allows the user to select a certain number by mouse.
 *
 * \sa KRatingPainter
 *
 * \author Sebastian Trueg <trueg@kde.org>
 */
class KWIDGETSADDONS_EXPORT KRatingWidget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int rating READ rating WRITE setRating)
    Q_PROPERTY(int maxRating READ maxRating WRITE setMaxRating)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool halfStepsEnabled READ halfStepsEnabled WRITE setHalfStepsEnabled)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

public:
    /**
     * Creates a new rating widget.
     */
    explicit KRatingWidget(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~KRatingWidget() override;

    /**
     * \return The current rating.
     */
    int rating() const;

    /**
     * \return the maximum possible rating.
     */
    int maxRating() const;

    /**
     * The alignment of the stars.
     *
     * \sa setAlignment
     */
    Qt::Alignment alignment() const;

    /**
     * The layout direction. If RTL the stars
     * representing the rating value will be drawn from the
     * right.
     *
     * \sa setLayoutDirection
     */
    Qt::LayoutDirection layoutDirection() const;

    /**
     * The spacing between the rating stars.
     *
     * \sa setSpacing
     */
    int spacing() const;

    QSize sizeHint() const override;

    /**
     * If half steps are enabled one star equals to 2 rating
     * points and uneven rating values result in half-stars being
     * drawn.
     *
     * \sa setHalfStepsEnabled
     */
    bool halfStepsEnabled() const;

    /**
     * The icon used to draw a star. In case a custom pixmap has been set
     * this value is ignored.
     *
     * \sa setIcon, setCustomPixmap
     */
    QIcon icon() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the rating is changed.
     */
    void ratingChanged(int rating);

public Q_SLOTS:
    /**
     * Set the current rating. Calling this method will trigger the
     * ratingChanged signal if @p rating is different from the previous rating.
     */
    void setRating(int rating);

    /**
     * Set the maximum allowed rating value. The default is 10 which means
     * that a rating from 1 to 10 is selectable. If \a max is uneven steps
     * are automatically only allowed full.
     */
    void setMaxRating(int max);

    /**
     * If half steps are enabled (the default) then
     * one rating step corresponds to half a star.
     */
    void setHalfStepsEnabled(bool enabled);

    /**
     * Set the spacing between the pixmaps. The default is 0.
     */
    void setSpacing(int);

    /**
     * The alignment of the stars in the drawing rect.
     * All alignment flags are supported.
     */
    void setAlignment(Qt::Alignment align);

    /**
     * LTR or RTL
     */
    void setLayoutDirection(Qt::LayoutDirection direction);

    /**
     * Set a custom icon. Defaults to "rating".
     */
    void setIcon(const QIcon &icon);

    /**
     * Set a custom pixmap.
     */
    void setCustomPixmap(const QPixmap &pixmap);

    /**
     * Set the recommended size of the pixmaps. This is
     * only used for the sizeHint. The actual size is always
     * dependent on the size of the widget itself.
     */
    void setPixmapSize(int size);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    std::unique_ptr<class KRatingWidgetPrivate> const d;
};

#endif
