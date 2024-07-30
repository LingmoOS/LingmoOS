/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Olaf Schmidt <ojschmidt@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGUIADDONS_COLORHELPERS_P_H
#define KGUIADDONS_COLORHELPERS_P_H

// normalize: like qBound(a, 0.0, 1.0) but without needing the args and with
// "safer" behavior on NaN (isnan(a) -> return 0.0)
static inline qreal normalize(qreal a)
{
    return (a < 1.0 ? (a > 0.0 ? a : 0.0) : 1.0);
}

#endif // KGUIADDONS_KCOLORHELPERS_P_H
