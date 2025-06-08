/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>


    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osdaction.h"

#include <KLocalizedString>

using namespace KScreen;

QVector<OsdAction> OsdAction::availableActions()
{
    return {
        {SwitchToExternal, i18nd("kscreen_common", "Switch to external screen"), QStringLiteral("osd-shutd-laptop")},
        {SwitchToInternal, i18nd("kscreen_common", "Switch to laptop screen"), QStringLiteral("osd-shutd-screen")},
        {Clone, i18nd("kscreen_common", "Unify outputs"), QStringLiteral("osd-duplicate")},
        {ExtendLeft, i18nd("kscreen_common", "Extend to left"), QStringLiteral("osd-sbs-left")},
        {ExtendRight, i18nd("kscreen_common", "Extend to right"), QStringLiteral("osd-sbs-sright")},
        {NoAction, i18nd("kscreen_common", "Leave unchanged"), QStringLiteral("dialog-cancel")},
    };
}
