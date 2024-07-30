/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LOGIND_INHIBITION_ARGUMENT_H
#define LOGIND_INHIBITION_ARGUMENT_H

#include "solid/power/power.h"
/**
 * The single responsibility of this class is to create arguments valid for
 * logind Inhibit call.
 */
namespace Solid
{
class LogindInhibitionArgument
{
public:
    static QString fromPowerState(Power::InhibitionTypes states);
};
}
#endif // LOGIND_INHIBITION_ARGUMENT_H
