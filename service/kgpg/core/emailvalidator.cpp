/*
    SPDX-FileCopyrightText: 2006 Jimmy Gilles <jimmygilles@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "emailvalidator.h"

#include <KEmailAddress>

namespace KgpgCore
{

QValidator::State EmailValidator::validate(QString &input, int &) const
{
    if (KEmailAddress::isValidSimpleAddress(input))
        return QValidator::Acceptable;
    else
        return QValidator::Invalid;
}

} // namespace KgpgCore
