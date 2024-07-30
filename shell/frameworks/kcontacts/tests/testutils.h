/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 KDE-PIM team <kde-pim@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <QByteArray>
#include <QString>

#include <kcontacts/addressee.h>

KContacts::Addressee vcard1();
KContacts::Addressee vcard2();
KContacts::Addressee vcard3();
KContacts::Addressee::List vCardsAsAddresseeList();
QByteArray vCardsAsText();

#endif
