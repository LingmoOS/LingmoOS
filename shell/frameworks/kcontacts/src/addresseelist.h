/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2002 Jost Schenck <jost@schenck.de>
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_ADDRESSEELIST_H
#define KCONTACTS_ADDRESSEELIST_H

#include <QList>

namespace KContacts
{
class Addressee;

/**
 * @short  a QList of Addressee.
 */
typedef QList<Addressee> AddresseeList;
}

#endif
