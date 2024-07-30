/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006-2007 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kratingwidget.h"
#include "kratingpainter.h"

#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

class KRatingWidgetPrivate
{
public:
    int rating = 0;
    int hoverRating = -1;
    int pixSize = 16;

    KRatingPainter ratingPainter;
};

KRatingWidget::KRatingWidget(QWidget *parent)
    : QFrame(parent)
    , d(new KRatingWidgetPrivate())
{
    setMouseTracking(true);
}

KRatingWidget::~KRatingWidget() = default;

void KRatingWidget::setCustomPixmap(const QPixmap &pix)
{
    d->ratingPainter.setCustomPixmap(pix);
    update();
}

void KRatingWidget::setIcon(const QIcon &icon)
{
    d->ratingPainter.setIcon(icon);
    update();
}

void KRatingWidget::setPixmapSize(int size)
{
    d->pixSize = size;
    updateGeometry();
}

int KRatingWidget::spacing() const
{
    return d->ratingPainter.spacing();
}

QIcon KRatingWidget::icon() const
{
    return d->ratingPainter.icon();
}

void KRatingWidget::setSpacing(int s)
{
    d->ratingPainter.setSpacing(s);
    update();
}

Qt::Alignment KRatingWidget::alignment() const
{
    return d->ratingPainter.alignment();
}

void KRatingWidget::setAlignment(Qt::Alignment align)
{
    d->ratingPainter.setAlignment(align);
    update();
}

Qt::LayoutDirection KRatingWidget::layoutDirection() const
{
    return d->ratingPainter.layoutDirection();
}

void KRatingWidget::setLayoutDirection(Qt::LayoutDirection direction)
{
    d->ratingPainter.setLayoutDirection(direction);
    update();
}

int KRatingWidget::rating() const
{
    return d->rating;
}

int KRatingWidget::maxRating() const
{
    return d->ratingPainter.maxRating();
}

bool KRatingWidget::halfStepsEnabled() const
{
    return d->ratingPainter.halfStepsEnabled();
}

void KRatingWidget::setRating(int rating)
{
    if (rating != d->rating) {
        d->rating = rating;
        d->hoverRating = rating;
        Q_EMIT ratingChanged(rating);
        update();
    }
}

void KRatingWidget::setMaxRating(int max)
{
    d->ratingPainter.setMaxRating(max);
    update();
}

void KRatingWidget::setHalfStepsEnabled(bool enabled)
{
    d->ratingPainter.setHalfStepsEnabled(enabled);
    update();
}

static inline int adjustedHoverRating(bool halfStep, int hoverRating, int rating)
{
    // intentionally skip zero, or half step is disabled.
    if (!halfStep || hoverRating == 0) {
        return hoverRating;
    }

    // See bug 171343, if we click on a star we want it to be full star, click again
    // make it half, click third time make it clear.

    // round up hoverRating to next full star rating
    const int hoveredFullStarRating = hoverRating + (hoverRating % 2);
    // Check if the star under mouse is the last half or whole star of the rating.
    if (hoveredFullStarRating == rating || hoveredFullStarRating == rating + 1) {
        // If current pointed star is not empty, next rating will be rating - 1
        // if we point at 4th star and rating is 8 (4 star), next click will make it 7
        // if we point at 4th star and rating is 7 (3.5 star), next click will make it 6
        hoverRating = rating - 1;
    } else {
        // otherwise make it a full star rating
        hoverRating = hoveredFullStarRating;
    }
    return hoverRating;
}

void KRatingWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        d->hoverRating = adjustedHoverRating(halfStepsEnabled(), d->ratingPainter.ratingFromPosition(contentsRect(), e->pos()), d->rating);
        // avoid set a rating to something less than zero, it may happen if widget is scaled and
        // mouse is clicked outside the star region.
        if (d->hoverRating >= 0) {
            setRating(d->hoverRating);
        }
    }
}

void KRatingWidget::mouseMoveEvent(QMouseEvent *e)
{
    // when moving the mouse we show the user what the result of clicking will be
    const int prevHoverRating = d->hoverRating;
    d->hoverRating = adjustedHoverRating(halfStepsEnabled(), d->ratingPainter.ratingFromPosition(contentsRect(), e->pos()), d->rating);
    if (d->hoverRating != prevHoverRating) {
        update();
    }
}

void KRatingWidget::leaveEvent(QEvent *)
{
    d->hoverRating = -1;
    update();
}

void KRatingWidget::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    QPainter p(this);
    d->ratingPainter.setEnabled(isEnabled());
    d->ratingPainter.paint(&p, contentsRect(), d->rating, d->hoverRating);
}

QSize KRatingWidget::sizeHint() const
{
    int numPix = d->ratingPainter.maxRating();
    if (d->ratingPainter.halfStepsEnabled()) {
        numPix /= 2;
    }

    QSize pixSize(d->pixSize, d->pixSize);
    if (!d->ratingPainter.customPixmap().isNull()) {
        pixSize = d->ratingPainter.customPixmap().size() / d->ratingPainter.customPixmap().devicePixelRatio();
    }

    return QSize(pixSize.width() * numPix + spacing() * (numPix - 1) + frameWidth() * 2, pixSize.height() + frameWidth() * 2);
}

void KRatingWidget::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
}

#include "moc_kratingwidget.cpp"
