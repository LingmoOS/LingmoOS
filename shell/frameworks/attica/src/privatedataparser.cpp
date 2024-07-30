/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "privatedataparser.h"

using namespace Attica;

PrivateData PrivateData::Parser::parseXml(QXmlStreamReader &xml)
{
    PrivateData data;
    QString key;

    // TODO: when we get internet and some documentation
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("key")) {
                key = xml.readElementText();
            } else if (xml.name() == QLatin1String("value")) {
                data.setAttribute(key, xml.readElementText());
            } else if (xml.name() == QLatin1String("timestamp")) {
                data.setTimestamp(key, QDateTime::fromString(xml.readElementText(), Qt::ISODate));
            }
        } else if (xml.isEndElement() && (xml.name() == QLatin1String("data") || xml.name() == QLatin1String("user"))) {
            break;
        }
    }

    return data;
}

QStringList PrivateData::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("privatedata"));
}
