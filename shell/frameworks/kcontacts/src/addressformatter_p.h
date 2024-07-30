/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_ADDRESSFORMATTER_H
#define KCONTACTS_ADDRESSFORMATTER_H

#include "namespace.h"

class QString;

namespace KContacts
{

class Address;
class AddressFormat;

/** Apply address formatting rules for a given address. */
namespace AddressFormatter
{
QString format(const Address &address, const QString &name, const QString &organization, const AddressFormat &format, AddressFormatStyle style);
};

}

#endif // KCONTACTS_ADDRESSFORMATTER_H
