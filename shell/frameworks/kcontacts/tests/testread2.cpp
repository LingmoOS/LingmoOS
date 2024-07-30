/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 KDE-PIM team <kde-pim@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testutils.h"
#include <QDebug>
#include <converter/kcontacts/vcardconverter.h>
#include <kcontacts/addressee.h>

#include <stdlib.h>

using namespace KContacts;

int main(int, char**)
{
    Addressee::List l = vCardsAsAddresseeList();
    QByteArray vcards = vCardsAsText();

    VCardConverter vct;

    Addressee::List parsed = vct.parseVCards(vcards);

    if (l.size() != parsed.size()) {
        qDebug() << "\tSize - FAILED :" << l.size() << "vs. parsed" << parsed.size();
    } else {
        qDebug() << "\tSize - PASSED";
    }

    Addressee::List::iterator itr1;
    Addressee::List::iterator itr2;
    // clang-format off
    for (itr1 = l.begin(), itr2 = parsed.begin();
         itr1 != l.end() && itr2 != parsed.end(); ++itr1, ++itr2) {
        if ((*itr1).fullEmail() == (*itr2).fullEmail()
            && (*itr1).organization() == (*itr2).organization()
            && (*itr1).phoneNumbers() == (*itr2).phoneNumbers()
            && (*itr1).emails() == (*itr2).emails()
            && (*itr1).role() == (*itr2).role()) {
            qDebug() << "\tAddressee  - PASSED";
            qDebug() << "\t\t" << (*itr1).fullEmail() << "VS." << (*itr2).fullEmail();
            // clang-format on
        } else {
            qDebug() << "\tAddressee  - FAILED";
            qDebug() << (*itr1).toString();
            qDebug() << (*itr2).toString();
            // qDebug()<<"\t\t"<< (*itr1).fullEmail() << "VS." << (*itr2).fullEmail();
        }
    }
    return EXIT_SUCCESS;
}
