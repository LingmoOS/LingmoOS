/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "kcolorutilssingleton.h"
#include <KColorUtils>

KColorUtilsSingleton::KColorUtilsSingleton(QObject *parent)
    : QObject(parent)
{
}

qreal KColorUtilsSingleton::hue(const QColor &color)
{
    return KColorUtils::hue(color);
}

qreal KColorUtilsSingleton::chroma(const QColor &color)
{
    return KColorUtils::chroma(color);
}

qreal KColorUtilsSingleton::luma(const QColor &color)
{
    return KColorUtils::luma(color);
}

qreal KColorUtilsSingleton::contrastRatio(const QColor &color1, const QColor &color2)
{
    return KColorUtils::contrastRatio(color1, color2);
}

QColor KColorUtilsSingleton::hcyColor(qreal hue, qreal chroma, qreal luma, qreal alpha)
{
    return KColorUtils::hcyColor(hue, chroma, luma, alpha);
}

QColor KColorUtilsSingleton::lighten(const QColor &color, qreal amount, qreal chromaInverseGain)
{
    return KColorUtils::lighten(color, amount, chromaInverseGain);
}

QColor KColorUtilsSingleton::darken(const QColor &color, qreal amount, qreal chromaGain)
{
    return KColorUtils::darken(color, amount, chromaGain);
}

QColor KColorUtilsSingleton::shade(const QColor &color, qreal lumaAmount, qreal chromaAmount)
{
    return KColorUtils::shade(color, lumaAmount, chromaAmount);
}

QColor KColorUtilsSingleton::tint(const QColor &base, const QColor &color, qreal amount)
{
    return KColorUtils::tint(base, color, amount);
}

QColor KColorUtilsSingleton::mix(const QColor &color1, const QColor &color2, qreal bias)
{
    return KColorUtils::mix(color1, color2, bias);
}

#include "moc_kcolorutilssingleton.cpp"
