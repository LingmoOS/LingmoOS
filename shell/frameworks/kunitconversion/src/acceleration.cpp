/*
 *   SPDX-FileCopyrightText: 2010 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "acceleration_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory Acceleration::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(AccelerationCategory, i18n("Acceleration"), i18n("Acceleration"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (acceleration)", "%1 %2");

    d->addDefaultUnit(UnitPrivate::makeUnit(AccelerationCategory,
                              MetresPerSecondSquared,
                              1,
                              i18nc("acceleration unit symbol", "m/s²"),
                              i18nc("unit description in lists", "meters per second squared"),
                              i18nc("unit synonyms for matching user input", "meter per second squared;meters per second squared;m/s²;m/s2;m/s^2"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 meters per second squared"),
                              ki18ncp("amount in units (integer)", "%1 meter per second squared", "%1 meters per second squared")));

    d->addCommonUnit(UnitPrivate::makeUnit(AccelerationCategory,
                             FeetPerSecondSquared,
                             0.3048,
                             i18nc("acceleration unit symbol", "ft/s²"),
                             i18nc("unit description in lists", "feet per second squared"),
                             i18nc("unit synonyms for matching user input", "foot per second squared;feet per second squared;ft/s²;ft/s2;ft/s^2"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 feet per second squared"),
                             ki18ncp("amount in units (integer)", "%1 foot per second squared", "%1 feet per second squared")));

    d->addUnit(UnitPrivate::makeUnit(AccelerationCategory,
                       StandardGravity,
                       9.80665,
                       i18nc("acceleration unit symbol", "g"),
                       i18nc("unit description in lists", "standard gravity"),
                       i18nc("unit synonyms for matching user input", "standard gravity;g"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 times standard gravity"),
                       ki18ncp("amount in units (integer)", "%1 standard gravity", "%1 times standard gravity")));

    return c;
}

} // KUnitConversion namespace
