/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KICONUTILS_H
#define KICONUTILS_H

#include <kguiaddons_export.h>

#include <QIcon>

/**
 * @namespace KIconUtils
 * Provides utility functions for icons.
 */
namespace KIconUtils
{
/**
 * Adds the \a overlay over the \a icon in the specified \a position
 *
 * The \a overlay icon is scaled down approx. to 1/3 or 1/4 (depending on the icon size)
 * and placed in one of the corners of the base icon.
 */
KGUIADDONS_EXPORT QIcon addOverlay(const QIcon &icon, const QIcon &overlay, Qt::Corner position);

/**
 * Adds \a overlays over the \a icon
 *
 * The \a overlays is a QHash of Qt::Corner and QIcon. The Qt::Corner value
 * decides where the overlay icon will be painted, the QIcon value
 * is the overlay icon to be painted.
 *
 * The overlay icon is scaled down to 1/3 or 1/4 (depending on the icon size)
 * and placed in one of the corners of the base icon.
 */
KGUIADDONS_EXPORT QIcon addOverlays(const QIcon &icon, const QHash<Qt::Corner, QIcon> &overlays);

/**
 * Adds up to four overlays over the @p icon.
 *
 * The @p overlays is a QStringList of icon names (e.g. the emblems that are drawn on
 * icons in Dolphin and KFileWidget, e.g. symlink, un-mounted device ...etc).
 *
 * Overlays are added in this order:
 * - first icon is used to paint an overlay on the bottom-right corner
 * - second icon on the bottom-left corner
 * - third icon on the top-left corner
 * - fourth icon on the top-right corner
 *
 * Each overlay icon is scaled down to 1/3 or 1/4 (depending on the icon size).
 *
 * @since 5.90
 */
KGUIADDONS_EXPORT QIcon addOverlays(const QIcon &icon, const QStringList &overlays);

/**
 * Adds up to four overlays on the icon constructed from @p iconName.
 *
 * The @p overlays is a QStringList of icon names (e.g. the emblems that are drawn on
 * icons in Dolphin and KFileWidget, e.g. symlink, un-mounted device ...etc).
 *
 * Overlays are added in this order:
 * - first icon is used to paint an overlay on the bottom-right corner
 * - second icon on the bottom-left corner
 * - third icon on the top-left corner
 * - fourth icon on the top-right corner
 *
 * Each overlay icon is scaled down to 1/3 or 1/4 (depending on the icon size).
 *
 * All @c QIcon objects are constructed using @c QIcon::fromTheme().
 *
 * @since 5.82
 */
KGUIADDONS_EXPORT QIcon addOverlays(const QString &iconName, const QStringList &overlays);
}

#endif // KICONUTILS_H
