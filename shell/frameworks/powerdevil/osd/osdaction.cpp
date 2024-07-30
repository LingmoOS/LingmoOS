/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>


    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osdaction.h"

#include <KLocalizedString>

using namespace PowerDevil;

QList<OsdAction> OsdAction::availableActions()
{
    return {
        {QStringLiteral("power-saver"), i18nc("power profile", "Power Save Mode"), QStringLiteral("battery-profile-powersave")},
        {QStringLiteral("balanced"), i18nc("power profile", "Balanced Performance Mode"), QStringLiteral("speedometer")},
        {QStringLiteral("performance"), i18nc("power profile", "Maximum Performance Mode"), QStringLiteral("battery-profile-performance")},
    };
}

#include "moc_osdaction.cpp"
