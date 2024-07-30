/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VCARDPARSER_H
#define VCARDPARSER_H

#include "vcard_p.h"
#include <QByteArray>

namespace KContacts
{
class VCardParser
{
public:
    VCardParser();
    ~VCardParser();

    static VCard::List parseVCards(const QByteArray &text);
    static QByteArray createVCards(const VCard::List &list);

private:
    Q_DISABLE_COPY(VCardParser)
    class VCardParserPrivate;
    VCardParserPrivate *d = nullptr;
};
}

#endif
