/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "oceanicons.h"

#include <QIcon>

namespace OceanIcons
{

void initIcons()
{
    // ensure we fallback to ocean, if no user fallback is set
    const QString fallbackTheme = QIcon::fallbackThemeName();
    if (fallbackTheme.isEmpty() || fallbackTheme == QLatin1String("hicolor")) {
        QIcon::setFallbackThemeName(QStringLiteral("ocean"));
    }
}

}
