/*
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPIXMAPSEQUENCELOADER_H
#define KPIXMAPSEQUENCELOADER_H

#include <KPixmapSequence>

#include "kiconwidgets_export.h"

namespace KPixmapSequenceLoader
{

/**
 * Loads a pixmapSequence given the xdg icon name
 *
 * @param iconName The name of the icon, without extension.
 * @param size the size to be used
 * @since 6.0
 */
KICONWIDGETS_EXPORT KPixmapSequence load(const QString &iconName, int size);

};

#endif
