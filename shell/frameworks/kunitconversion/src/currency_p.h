/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef CURRENCY_P_H
#define CURRENCY_P_H

#include "unitcategory_p.h"
#include <QDateTime>

namespace KUnitConversion
{
namespace Currency
{
    UnitCategory makeCategory();
    /**
     * @brief Provides time of last conversion table update for usage in tests
     *
     * If no conversion table is available the QDateTime object is a null datetime.
     *
     * @internal exported for unit tests only
     */
    KUNITCONVERSION_EXPORT QDateTime lastConversionTableUpdate();
};

}

#endif
