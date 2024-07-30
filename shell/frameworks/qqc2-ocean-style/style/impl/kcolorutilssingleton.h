/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef KCOLORUTILSSINGLETON_H
#define KCOLORUTILSSINGLETON_H

#include <QColor>
#include <QObject>
#include <qqml.h>

class KColorUtilsSingleton : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(KColorUtils)
    QML_SINGLETON

public:
    explicit KColorUtilsSingleton(QObject *parent = nullptr);
    Q_INVOKABLE qreal hue(const QColor &color);
    Q_INVOKABLE qreal chroma(const QColor &color);
    Q_INVOKABLE qreal luma(const QColor &color);
    Q_INVOKABLE QColor hcyColor(qreal hue, qreal chroma, qreal luma, qreal alpha = 1.0);
    Q_INVOKABLE qreal contrastRatio(const QColor &color1, const QColor &color2);
    Q_INVOKABLE QColor lighten(const QColor &color, qreal amount = 0.5, qreal chromaInverseGain = 1.0);
    Q_INVOKABLE QColor darken(const QColor &color, qreal amount = 0.5, qreal chromaGain = 1.0);
    Q_INVOKABLE QColor shade(const QColor &color, qreal lumaAmount, qreal chromaAmount = 0.0);
    Q_INVOKABLE QColor tint(const QColor &base, const QColor &color, qreal amount = 0.3);
    Q_INVOKABLE QColor mix(const QColor &color1, const QColor &color2, qreal bias = 0.5);
};

#endif // KCOLORUTILSSINGLETON_H
