/*
    SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "utils.h"

// Returns "hciX" part from UBI "/org/bluez/hciX/dev_xx_xx_xx_xx_xx_xx"
QString Utils::adapterHciString(const QString &ubi)
{
    int startIndex = ubi.indexOf(QLatin1String("/hci")) + 1;

    if (startIndex < 1) {
        return QString();
    }

    int endIndex = ubi.indexOf(QLatin1Char('/'), startIndex);

    if (endIndex == -1) {
        return ubi.mid(startIndex);
    }
    return ubi.mid(startIndex, endIndex - startIndex);
}

#include "moc_utils.cpp"
