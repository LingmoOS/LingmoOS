/*
 *   SPDX-FileCopyrightText: 2018 João Netto <joaonetto901@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "permeability_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory Permeability::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(PermeabilityCategory, i18n("Permeability"), i18n("Permeability"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (permeability)", "%1 %2");

    d->addDefaultUnit(UnitPrivate::makeUnit(PermeabilityCategory,
                              Darcy,
                              1,
                              i18nc("volume unit symbol", "Darcy"),
                              i18nc("unit description in lists", "Darcy"),
                              i18nc("unit synonyms for matching user input", "Darcy;Darcys;Dar;Darc"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 Darcy"),
                              ki18ncp("amount in units (integer)", "%1 Darcy", "%1 Darcys")));

    d->addCommonUnit(UnitPrivate::makeUnit(PermeabilityCategory,
                             MiliDarcy,
                             0.001,
                             i18nc("volume unit symbol", "mDarcy"),
                             i18nc("unit description in lists", "Milli-Darcy"),
                             i18nc("unit synonyms for matching user input", "Milli-Darcy;MilliDarcy;MilliDar;mDarcy;mDar;mDarc"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 Milli-Darcy"),
                             ki18ncp("amount in units (integer)", "%1 Milli-Darcy", "%1 Milli-Darcys")));

    d->addCommonUnit(UnitPrivate::makeUnit(PermeabilityCategory,
                             PermeabilitySquareMicrometer,
                             0.9869233,
                             i18nc("volume unit symbol", "µm²"),
                             i18nc("unit description in lists", "squaremicrometers"),
                             i18nc("unit synonyms for matching user input", "Permeability;Pµm²;PSquare µm;squaremicrometers;Pµm^2"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 micrometers²"),
                             ki18ncp("amount in units (integer)", "%1 micrometer²", "%1 micrometers²")));

    return c;
}

} // KUnitConversion namespace
