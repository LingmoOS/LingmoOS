/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_ENGINE_FSUTILS_H
#define BALOO_ENGINE_FSUTILS_H

#include <QString>

namespace Baloo {
namespace FSUtils {

/**
 * Disables filesystem copy-on-write feature on given file or directory.
 * Only implemented on Linux and does nothing on other platforms.
 */
void disableCoW(const QString &path);

}
}
#endif
