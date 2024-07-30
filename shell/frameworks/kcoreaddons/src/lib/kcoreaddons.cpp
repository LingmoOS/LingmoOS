/*
    This file is part of the KDE Libraries

    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcoreaddons.h"

#include "kcoreaddons_version.h"

QString KCoreAddons::versionString()
{
    return QStringLiteral(KCOREADDONS_VERSION_STRING);
}

uint KCoreAddons::version()
{
    return KCOREADDONS_VERSION;
}
