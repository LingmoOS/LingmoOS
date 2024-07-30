/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "accountbalanceparser.h"

#include <QXmlStreamReader>

using namespace Attica;

QStringList AccountBalance::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("person"));
}

AccountBalance AccountBalance::Parser::parseXml(QXmlStreamReader &xml)
{
    AccountBalance item;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("balance")) {
                item.setBalance(xml.readElementText());
            } else if (xml.name() == QLatin1String("currency")) {
                item.setCurrency(xml.readElementText());
            }
        }
    }
    return item;
}
