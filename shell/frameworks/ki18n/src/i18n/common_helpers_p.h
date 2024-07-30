/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COMMON_HELPERS_P_H
#define COMMON_HELPERS_P_H

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

#endif
