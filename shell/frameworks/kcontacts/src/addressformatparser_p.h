/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_ADDRESSFORMATPARSER_P_H
#define KCONTACTS_ADDRESSFORMATPARSER_P_H

#include "addressformat.h"

#include <vector>

namespace KContacts
{

/** Parsing functions for address format data.
 *  @internal
 */
namespace AddressFormatParser
{
AddressFormatField parseField(QChar c);
AddressFormatFields parseFields(QStringView s);
std::vector<AddressFormatElement> parseElements(QStringView s);
}
}

#endif // KCONTACTS_ADDRESSFORMATPARSER_P_H
