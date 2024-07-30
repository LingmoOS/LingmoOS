/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
    SPDX-FileCopyrightText: 2007 Zack Rusin <zack@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kcolorspaces_p.h"
#include "kguiaddons_colorhelpers_p.h"
#include <kcolorutils.h>

#include <QColor>
#include <QImage>
#include <QtNumeric> // qIsNaN

#include <math.h>

// BEGIN internal helper functions
static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}
// END internal helper functions

qreal KColorUtils::hue(const QColor &color)
{
    return KColorSpaces::KHCY::hue(color);
}

qreal KColorUtils::chroma(const QColor &color)
{
    return KColorSpaces::KHCY::chroma(color);
}

qreal KColorUtils::luma(const QColor &color)
{
    return KColorSpaces::KHCY::luma(color);
}

void KColorUtils::getHcy(const QColor &color, qreal *h, qreal *c, qreal *y, qreal *a)
{
    if (!c || !h || !y) {
        return;
    }
    KColorSpaces::KHCY khcy(color);
    *c = khcy.c;
    *h = khcy.h + (khcy.h < 0.0 ? 1.0 : 0.0);
    *y = khcy.y;
    if (a) {
        *a = khcy.a;
    }
}

QColor KColorUtils::hcyColor(qreal h, qreal c, qreal y, qreal a)
{
    return KColorSpaces::KHCY(h, c, y, a).qColor();
}

static qreal contrastRatioForLuma(qreal y1, qreal y2)
{
    if (y1 > y2) {
        return (y1 + 0.05) / (y2 + 0.05);
    } else {
        return (y2 + 0.05) / (y1 + 0.05);
    }
}

qreal KColorUtils::contrastRatio(const QColor &c1, const QColor &c2)
{
    return contrastRatioForLuma(luma(c1), luma(c2));
}

QColor KColorUtils::lighten(const QColor &color, qreal ky, qreal kc)
{
    KColorSpaces::KHCY c(color);
    c.y = 1.0 - normalize((1.0 - c.y) * (1.0 - ky));
    c.c = 1.0 - normalize((1.0 - c.c) * kc);
    return c.qColor();
}

QColor KColorUtils::darken(const QColor &color, qreal ky, qreal kc)
{
    KColorSpaces::KHCY c(color);
    c.y = normalize(c.y * (1.0 - ky));
    c.c = normalize(c.c * kc);
    return c.qColor();
}

QColor KColorUtils::shade(const QColor &color, qreal ky, qreal kc)
{
    KColorSpaces::KHCY c(color);
    c.y = normalize(c.y + ky);
    c.c = normalize(c.c + kc);
    return c.qColor();
}

static KColorSpaces::KHCY tintHelper(const QColor &base, qreal baseLuma, const QColor &color, qreal amount)
{
    KColorSpaces::KHCY result(KColorUtils::mix(base, color, pow(amount, 0.3)));
    result.y = mixQreal(baseLuma, result.y, amount);

    return result;
}

static qreal tintHelperLuma(const QColor &base, qreal baseLuma, const QColor &color, qreal amount)
{
    qreal result(KColorUtils::luma(KColorUtils::mix(base, color, pow(amount, 0.3))));
    result = mixQreal(baseLuma, result, amount);

    return result;
}

QColor KColorUtils::tint(const QColor &base, const QColor &color, qreal amount)
{
    if (amount <= 0.0) {
        return base;
    }
    if (amount >= 1.0) {
        return color;
    }
    if (qIsNaN(amount)) {
        return base;
    }

    qreal baseLuma = luma(base); // cache value because luma call is expensive
    double ri = contrastRatioForLuma(baseLuma, luma(color));
    double rg = 1.0 + ((ri + 1.0) * amount * amount * amount);
    double u = 1.0;
    double l = 0.0;
    double a = 0.5;
    for (int i = 12; i; --i) {
        a = 0.5 * (l + u);
        qreal resultLuma = tintHelperLuma(base, baseLuma, color, a);
        double ra = contrastRatioForLuma(baseLuma, resultLuma);
        if (ra > rg) {
            u = a;
        } else {
            l = a;
        }
    }
    return tintHelper(base, baseLuma, color, a).qColor();
}

QColor KColorUtils::mix(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);
    if (a <= 0.0) {
        return Qt::transparent;
    }

    qreal r = qBound(0.0, mixQreal(c1.redF() * c1.alphaF(), c2.redF() * c2.alphaF(), bias), 1.0) / a;
    qreal g = qBound(0.0, mixQreal(c1.greenF() * c1.alphaF(), c2.greenF() * c2.alphaF(), bias), 1.0) / a;
    qreal b = qBound(0.0, mixQreal(c1.blueF() * c1.alphaF(), c2.blueF() * c2.alphaF(), bias), 1.0) / a;

    return QColor::fromRgbF(r, g, b, a);
}

QColor KColorUtils::overlayColors(const QColor &base, const QColor &paint, QPainter::CompositionMode comp)
{
    // This isn't the fastest way, but should be "fast enough".
    // It's also the only safe way to use QPainter::CompositionMode
    QImage img(1, 1, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    QColor start = base;
    start.setAlpha(255); // opaque
    p.fillRect(0, 0, 1, 1, start);
    p.setCompositionMode(comp);
    p.fillRect(0, 0, 1, 1, paint);
    p.end();
    return img.pixel(0, 0);
}
