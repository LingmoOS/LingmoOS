/*
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kpixmapsequenceloader.h"

#include <KIconLoader>

namespace KPixmapSequenceLoader
{

KPixmapSequence load(const QString &iconName, int size)
{
    return KPixmapSequence(KIconLoader::global()->iconPath(iconName, -size), size);
}

}
