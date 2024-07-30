/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
 * If you use KColorSpaces in your own KDE code, please drop me a line at
 * mw_triad@users.sourceforge.net, as I would like to track if people find it
 * useful. Thanks!
 */

#ifndef KCOLORSPACES_H
#define KCOLORSPACES_H

#include <QColor>

namespace KColorSpaces
{
class KHCY
{
public:
    explicit KHCY(const QColor &);
    explicit KHCY(qreal h_, qreal c_, qreal y_, qreal a_ = 1.0);
    QColor qColor() const;
    qreal h, c, y, a;
    static qreal hue(const QColor &);
    static qreal chroma(const QColor &);
    static qreal luma(const QColor &);

private:
    static qreal gamma(qreal);
    static qreal igamma(qreal);
    static qreal lumag(qreal, qreal, qreal);
};

}

#endif
