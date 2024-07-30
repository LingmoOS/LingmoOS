/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007-2008 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kratingpainter.h"

#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QRect>

class KRatingPainterPrivate
{
public:
    QPixmap getPixmap(int size, QIcon::State state = QIcon::On);

    int maxRating = 10;
    int spacing = 0;
    QIcon icon;
    bool isEnabled = true;
    bool bHalfSteps = true;
    Qt::Alignment alignment = Qt::AlignCenter;
    Qt::LayoutDirection direction = Qt::LeftToRight;
    QPixmap customPixmap;
};

static void imageToGrayScale(QImage &img, float value);
static void imageToSemiTransparent(QImage &img);

QPixmap KRatingPainterPrivate::getPixmap(int size, QIcon::State state)
{
    bool transformToOffState = (state == QIcon::Off);
    QPixmap p;

    if (!customPixmap.isNull()) {
        p = customPixmap.scaled(QSize(size, size));
    } else {
        QIcon _icon(icon);
        if (_icon.isNull()) {
            if (state == QIcon::On) {
                _icon = QIcon::fromTheme(QStringLiteral("rating"));
            } else if (QIcon::hasThemeIcon(QStringLiteral("rating-unrated"))) {
                _icon = QIcon::fromTheme(QStringLiteral("rating-unrated"));
                transformToOffState = false; // no need because we already have the perfect icon
            } else {
                _icon = QIcon::fromTheme(QStringLiteral("rating")); // will be transformed to the "off" state
            }
        }
        p = _icon.pixmap(size);
    }

    if (transformToOffState) {
        QImage img = p.toImage().convertToFormat(QImage::Format_ARGB32);
        imageToGrayScale(img, 1.0);
        // The icon might have already been monochrome, so we also need to make it semi-transparent to see a difference.
        imageToSemiTransparent(img);
        return QPixmap::fromImage(img);
    }
    return p;
}

KRatingPainter::KRatingPainter()
    : d(new KRatingPainterPrivate())
{
}

KRatingPainter::~KRatingPainter() = default;

int KRatingPainter::maxRating() const
{
    return d->maxRating;
}

bool KRatingPainter::halfStepsEnabled() const
{
    return d->bHalfSteps;
}

Qt::Alignment KRatingPainter::alignment() const
{
    return d->alignment;
}

Qt::LayoutDirection KRatingPainter::layoutDirection() const
{
    return d->direction;
}

QIcon KRatingPainter::icon() const
{
    return d->icon;
}

bool KRatingPainter::isEnabled() const
{
    return d->isEnabled;
}

QPixmap KRatingPainter::customPixmap() const
{
    return d->customPixmap;
}

int KRatingPainter::spacing() const
{
    return d->spacing;
}

void KRatingPainter::setMaxRating(int max)
{
    d->maxRating = max;
}

void KRatingPainter::setHalfStepsEnabled(bool enabled)
{
    d->bHalfSteps = enabled;
}

void KRatingPainter::setAlignment(Qt::Alignment align)
{
    d->alignment = align;
}

void KRatingPainter::setLayoutDirection(Qt::LayoutDirection direction)
{
    d->direction = direction;
}

void KRatingPainter::setIcon(const QIcon &icon)
{
    d->icon = icon;
}

void KRatingPainter::setEnabled(bool enabled)
{
    d->isEnabled = enabled;
}

void KRatingPainter::setCustomPixmap(const QPixmap &pixmap)
{
    d->customPixmap = pixmap;
}

void KRatingPainter::setSpacing(int s)
{
    d->spacing = qMax(0, s);
}

static void imageToGrayScale(QImage &img, float value)
{
    QRgb *data = (QRgb *)img.bits();
    QRgb *end = data + img.width() * img.height();

    unsigned char gray;
    unsigned char val = (unsigned char)(255.0 * value);
    while (data != end) {
        gray = qGray(*data);
        *data = qRgba((val * gray + (255 - val) * qRed(*data)) >> 8,
                      (val * gray + (255 - val) * qGreen(*data)) >> 8,
                      (val * gray + (255 - val) * qBlue(*data)) >> 8,
                      qAlpha(*data));
        ++data;
    }
}

static void imageToSemiTransparent(QImage &img)
{
    QRgb *data = (QRgb *)img.bits();
    QRgb *end = data + img.width() * img.height();

    while (data != end) {
        *data = qRgba(qRed(*data), qGreen(*data), qBlue(*data), qAlpha(*data) >> 1);
        ++data;
    }
}

void KRatingPainter::paint(QPainter *painter, const QRect &rect, int rating, int hoverRating) const
{
    rating = qMin(rating, d->maxRating);
    hoverRating = qMin(hoverRating, d->maxRating);

    int numUsedStars = d->bHalfSteps ? d->maxRating / 2 : d->maxRating;

    if (hoverRating >= 0 && hoverRating < rating) {
        int tmp = hoverRating;
        hoverRating = rating;
        rating = tmp;
    }

    int usedSpacing = d->spacing;

    // get the rating pixmaps
    int maxHSizeOnePix = (rect.width() - (numUsedStars - 1) * usedSpacing) / numUsedStars;
    QPixmap ratingPix = d->getPixmap(qMin(rect.height(), maxHSizeOnePix), QIcon::On);
    QSize ratingPixSize = ratingPix.size() / ratingPix.devicePixelRatio();

    QPixmap disabledRatingPix = d->getPixmap(qMin(rect.height(), maxHSizeOnePix), QIcon::Off);
    QImage disabledRatingImage = disabledRatingPix.toImage().convertToFormat(QImage::Format_ARGB32);
    QPixmap hoverPix;

    // if we are disabled we become gray and more transparent
    if (!d->isEnabled) {
        ratingPix = disabledRatingPix;

        imageToSemiTransparent(disabledRatingImage);
        disabledRatingPix = QPixmap::fromImage(disabledRatingImage);
    }

    bool half = d->bHalfSteps && rating % 2;
    int numRatingStars = d->bHalfSteps ? rating / 2 : rating;

    int numHoverStars = 0;
    bool halfHover = false;
    if (hoverRating >= 0 && rating != hoverRating && d->isEnabled) {
        numHoverStars = d->bHalfSteps ? hoverRating / 2 : hoverRating;
        halfHover = d->bHalfSteps && hoverRating % 2;

        disabledRatingImage = ratingPix.toImage().convertToFormat(QImage::Format_ARGB32);
        imageToGrayScale(disabledRatingImage, 0.5);

        hoverPix = QPixmap::fromImage(disabledRatingImage);
    }

    if (d->alignment & Qt::AlignJustify && numUsedStars > 1) {
        int w = rect.width();
        w -= numUsedStars * ratingPixSize.width();
        usedSpacing = w / (numUsedStars - 1);
    }

    int ratingAreaWidth = ratingPixSize.width() * numUsedStars + usedSpacing * (numUsedStars - 1);

    int i = 0;
    int x = rect.x();
    if (d->alignment & Qt::AlignRight) {
        x += (rect.width() - ratingAreaWidth);
    } else if (d->alignment & Qt::AlignHCenter) {
        x += (rect.width() - ratingAreaWidth) / 2;
    }

    int xInc = ratingPixSize.width() + usedSpacing;
    if (d->direction == Qt::RightToLeft) {
        x = rect.width() - ratingPixSize.width() - x;
        xInc = -xInc;
    }

    int y = rect.y();
    if (d->alignment & Qt::AlignVCenter) {
        y += (rect.height() / 2 - ratingPixSize.height() / 2);
    } else if (d->alignment & Qt::AlignBottom) {
        y += (rect.height() - ratingPixSize.height());
    }
    for (; i < numRatingStars; ++i) {
        painter->drawPixmap(x, y, ratingPix);
        x += xInc;
    }
    if (half) {
        painter->drawPixmap(x,
                            y,
                            ratingPixSize.width() / 2,
                            ratingPixSize.height(),
                            d->direction == Qt::RightToLeft ? (numHoverStars > 0 ? hoverPix : disabledRatingPix) : ratingPix,
                            0,
                            0,
                            ratingPix.width() / 2,
                            ratingPix.height()); // source sizes are deliberately not device independent
        painter->drawPixmap(x + ratingPixSize.width() / 2,
                            y,
                            ratingPixSize.width() / 2,
                            ratingPixSize.height(),
                            d->direction == Qt::RightToLeft ? ratingPix : (numHoverStars > 0 ? hoverPix : disabledRatingPix),
                            ratingPix.width() / 2,
                            0,
                            ratingPix.width() / 2,
                            ratingPix.height());
        x += xInc;
        ++i;
    }
    for (; i < numHoverStars; ++i) {
        painter->drawPixmap(x, y, hoverPix);
        x += xInc;
    }
    if (halfHover) {
        painter->drawPixmap(x,
                            y,
                            ratingPixSize.width() / 2,
                            ratingPixSize.height(),
                            d->direction == Qt::RightToLeft ? disabledRatingPix : hoverPix,
                            0,
                            0,
                            ratingPix.width() / 2,
                            ratingPix.height());
        painter->drawPixmap(x + ratingPixSize.width() / 2,
                            y,
                            ratingPixSize.width() / 2,
                            ratingPixSize.height(),
                            d->direction == Qt::RightToLeft ? hoverPix : disabledRatingPix,
                            ratingPix.width() / 2,
                            0,
                            ratingPix.width() / 2,
                            ratingPix.height());
        x += xInc;
        ++i;
    }
    for (; i < numUsedStars; ++i) {
        painter->drawPixmap(x, y, disabledRatingPix);
        x += xInc;
    }
}

int KRatingPainter::ratingFromPosition(const QRect &rect, const QPoint &pos) const
{
    int usedSpacing = d->spacing;
    int numUsedStars = d->bHalfSteps ? d->maxRating / 2 : d->maxRating;
    int maxHSizeOnePix = (rect.width() - (numUsedStars - 1) * usedSpacing) / numUsedStars;
    QPixmap ratingPix = d->getPixmap(qMin(rect.height(), maxHSizeOnePix));
    QSize ratingPixSize = ratingPix.size() / ratingPix.devicePixelRatio();

    int ratingAreaWidth = ratingPixSize.width() * numUsedStars + usedSpacing * (numUsedStars - 1);

    QRect usedRect(rect);
    if (d->alignment & Qt::AlignRight) {
        usedRect.setLeft(rect.right() - ratingAreaWidth);
    } else if (d->alignment & Qt::AlignHCenter) {
        int x = (rect.width() - ratingAreaWidth) / 2;
        usedRect.setLeft(rect.left() + x);
        usedRect.setRight(rect.right() - x);
    } else { // d->alignment & Qt::AlignLeft
        usedRect.setRight(rect.left() + ratingAreaWidth - 1);
    }

    if (d->alignment & Qt::AlignBottom) {
        usedRect.setTop(rect.bottom() - ratingPixSize.height() + 1);
    } else if (d->alignment & Qt::AlignVCenter) {
        int x = (rect.height() - ratingPixSize.height()) / 2;
        usedRect.setTop(rect.top() + x);
        usedRect.setBottom(rect.bottom() - x);
    } else { // d->alignment & Qt::AlignTop
        usedRect.setBottom(rect.top() + ratingPixSize.height() - 1);
    }

    if (usedRect.contains(pos)) {
        int x = 0;
        if (d->direction == Qt::RightToLeft) {
            x = usedRect.right() - pos.x();
        } else {
            x = pos.x() - usedRect.left();
        }

        double one = (double)usedRect.width() / (double)d->maxRating;

        //        qCDebug(KWidgetsAddonsLog) << "rating:" << ( int )( ( double )x/one + 0.5 );

        return (int)((double)x / one + 0.5);
    } else {
        return -1;
    }
}

void KRatingPainter::paintRating(QPainter *painter, const QRect &rect, Qt::Alignment align, int rating, int hoverRating)
{
    KRatingPainter rp;
    rp.setAlignment(align);
    rp.setLayoutDirection(painter->layoutDirection());
    rp.paint(painter, rect, rating, hoverRating);
}

int KRatingPainter::getRatingFromPosition(const QRect &rect, Qt::Alignment align, Qt::LayoutDirection direction, const QPoint &pos)
{
    KRatingPainter rp;
    rp.setAlignment(align);
    rp.setLayoutDirection(direction);
    return rp.ratingFromPosition(rect, pos);
}
