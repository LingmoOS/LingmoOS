/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COMMON_HELPERS_P_H
#define COMMON_HELPERS_P_H

#include <QLocale>
#include <QString>

// Standalone (pure Qt) functionality needed internally in more than
// one source file on localization.

/**
 * @internal
 *
 * Removes accelerator marker from a UI text label.
 *
 * Accelerator marker is not always a plain ampersand (&),
 * so it is not enough to just remove it by @c QString::remove().
 * The label may contain escaped markers ("&&") which must be resolved
 * and skipped, as well as CJK-style markers ("Foo (&F)") where
 * the whole parenthesis construct should be removed.
 * Therefore always use this function to remove accelerator marker
 * from UI labels.
 *
 * @param label UI label which may contain an accelerator marker
 * @return label without the accelerator marker
 */
QString removeAcceleratorMarker(const QString &label);

/**
 * @internal
 *
 * Returns the date format used for the locale with the year as a four digit
 * number.
 *
 * If the date format used for the locale already has a 4-digit year, then
 * this is the same as locale.dateFormat(format). Otherwise, it's the result
 * of QLocale::dateFormat with "yy" replaced with "yyyy".
 * In particular when using the short format for date inputs you may want to
 * use the short format with 4-digit year because Qt parses current 2-digit year
 * values like 22 as 1922.
 *
 * @param locale The locale that you want the date format for.
 * @param format The date format that you want.
 * @return the date format string with 4-digit year
 */
QString dateFormatWith4DigitYear(const QLocale &locale, QLocale::FormatType format);

#endif
