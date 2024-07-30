/*
    SPDX-FileCopyrightText: 2005, 2009, 2014 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kfontutils.h"

#include <QPainter>
#include <qmath.h>

static bool checkFits(QPainter &painter, const QString &string, qreal width, qreal height, qreal size, KFontUtils::AdaptFontSizeOptions flags)
{
    QFont f = painter.font();
    f.setPointSizeF(size);
    painter.setFont(f);
    int qtFlags = Qt::AlignCenter | Qt::TextWordWrap;
    if (flags & KFontUtils::DoNotAllowWordWrap) {
        qtFlags &= ~Qt::TextWordWrap;
    }
    const QRectF boundingRect = painter.boundingRect(QRectF(0, 0, width, height), qtFlags, string);
    if (boundingRect.width() == 0.0 || boundingRect.height() == 0.0) {
        return false;
    } else if (boundingRect.width() > width || boundingRect.height() > height) {
        return false;
    }
    return true;
}

qreal KFontUtils::adaptFontSize(QPainter &painter,
                                const QString &string,
                                qreal width,
                                qreal height,
                                qreal maxFontSize,
                                qreal minFontSize,
                                AdaptFontSizeOptions flags)
{
    // A invalid range is an error (-1)
    if (maxFontSize < minFontSize) {
        return -1;
    }

    // If the max font size already fits, return it
    if (checkFits(painter, string, width, height, maxFontSize, flags)) {
        return maxFontSize;
    }

    qreal fontSizeDoesNotFit = maxFontSize;

    // If the min font size does not fit, try to see if a font size of 1 fits,
    // if it does not return error (-1)
    // if it does, we'll return a fontsize smaller than the minFontSize as documented
    if (!checkFits(painter, string, width, height, minFontSize, flags)) {
        fontSizeDoesNotFit = minFontSize;

        minFontSize = 1;
        if (!checkFits(painter, string, width, height, minFontSize, flags)) {
            return -1;
        }
    }

    qreal fontSizeFits = minFontSize;
    qreal nextFontSizeToTry = (fontSizeDoesNotFit + fontSizeFits) / 2;

    while (qFloor(fontSizeFits) != qFloor(nextFontSizeToTry)) {
        if (checkFits(painter, string, width, height, nextFontSizeToTry, flags)) {
            fontSizeFits = nextFontSizeToTry;
            nextFontSizeToTry = (fontSizeDoesNotFit + fontSizeFits) / 2;
        } else {
            fontSizeDoesNotFit = nextFontSizeToTry;
            nextFontSizeToTry = (nextFontSizeToTry + fontSizeFits) / 2;
        }
    }

    QFont f = painter.font();
    f.setPointSizeF(fontSizeFits);
    painter.setFont(f);

    return fontSizeFits;
}

qreal KFontUtils::adaptFontSize(QPainter &painter,
                                const QString &text,
                                const QSizeF &availableSize,
                                qreal maxFontSize,
                                qreal minFontSize,
                                AdaptFontSizeOptions flags)
{
    return adaptFontSize(painter, text, availableSize.width(), availableSize.height(), maxFontSize, minFontSize, flags);
}
