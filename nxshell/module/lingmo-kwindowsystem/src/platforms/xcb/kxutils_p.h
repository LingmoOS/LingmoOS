/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Lubos Lunak (l.lunak@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KXUTILS_H
#define KXUTILS_H

#include <QScopedPointer>
#include <QPixmap>
#include <config-kwindowsystem.h>

#if KWINDOWSYSTEM_HAVE_X11

#include <kwindowsystem_export.h>

struct xcb_connection_t;

/**
 * Namespace with various generic X11-related functionality.
 */
namespace KXUtils
{

template <typename T>
class ScopedCPointer : public QScopedPointer<T, QScopedPointerPodDeleter>
{
public:
    ScopedCPointer(T *p = nullptr) : QScopedPointer<T, QScopedPointerPodDeleter>(p) {}
};

/**
 * Creates a QPixmap that contains a copy of the pixmap given by the X handle @p pixmap
 * and optionally also mask given as another X handle @mask. This function tries to
 * also handle the case when the depth of the pixmap differs from the native QPixmap depth.
 * @since 4.0.2
 */
QPixmap createPixmapFromHandle(WId pixmap, WId mask = 0);
QPixmap createPixmapFromHandle(xcb_connection_t *c, WId pixmap, WId mask = 0);

/**
 * Compares two X timestamps, taking into account wrapping and 64bit architectures.
 * Return value is like with strcmp(), 0 for equal, -1 for time1 < time2, 1 for time1 > time2.
 * @since 4.1.0
 */
int timestampCompare(unsigned long time1, unsigned long time2);
/**
 * Returns a difference of two X timestamps, time2 - time1, where time2 must be later than time1,
 * as returned by timestampCompare().
 * @since 4.1.0
 */
int timestampDiff(unsigned long time1, unsigned long time2);

} // namespace

#endif // KWINDOWSYSTEM_HAVE_X11

#endif
