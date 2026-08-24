/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breeze.h"

namespace Breeze
{

QString lingmoLongVersion()
{
    QString version;

    version = QString(LINGMO_VERSION);
#if LINGMO_GIT_MASTER
    version += ".git";
#endif
    return version;
}

}
