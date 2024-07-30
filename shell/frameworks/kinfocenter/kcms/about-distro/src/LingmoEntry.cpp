/*
    SPDX-FileCopyrightText: 2012-2021 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "LingmoEntry.h"

#include <KConfigGroup>
#include <KDesktopFile>
#include <QStandardPaths>

#include "Version.h"

LingmoEntry::LingmoEntry()
    : Entry(ki18n("Lingmo Version:"), lingmoVersion())
{
    // Since Lingmo version detection isn't based on a library query it can fail
    // in weird cases; instead of admitting defeat we simply hide everything :P
    if (localizedValue().isEmpty()) {
        return;
    }
}

QString LingmoEntry::lingmoVersion()
{
    // KInfoCenter is part of Lingmo. Simply show our version!
    return QString::fromLatin1(PROJECT_VERSION);
}
