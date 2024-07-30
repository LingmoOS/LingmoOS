/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "personparser.h"

using namespace Attica;

Person Person::Parser::parseXml(QXmlStreamReader &xml)
{
    Person person;
    bool hasAvatarPic = false;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("personid")) {
                person.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("firstname")) {
                person.setFirstName(xml.readElementText());
            } else if (xml.name() == QLatin1String("lastname")) {
                person.setLastName(xml.readElementText());
            } else if (xml.name() == QLatin1String("homepage")) {
                person.setHomepage(xml.readElementText());
            } else if (xml.name() == QLatin1String("avatarpic")) {
                person.setAvatarUrl(QUrl(xml.readElementText()));
            } else if (xml.name() == QLatin1String("avatarpicfound")) {
                QString value = xml.readElementText();
                if (value.toInt()) {
                    hasAvatarPic = true;
                }
            } else if (xml.name() == QLatin1String("birthday")) {
                person.setBirthday(QDate::fromString(xml.readElementText(), Qt::ISODate));
            } else if (xml.name() == QLatin1String("city")) {
                person.setCity(xml.readElementText());
            } else if (xml.name() == QLatin1String("country")) {
                person.setCountry(xml.readElementText());
            } else if (xml.name() == QLatin1String("latitude")) {
                person.setLatitude(xml.readElementText().toFloat());
            } else if (xml.name() == QLatin1String("longitude")) {
                person.setLongitude(xml.readElementText().toFloat());
            } else {
                person.addExtendedAttribute(xml.name().toString(), xml.readElementText());
            }
        } else if (xml.isEndElement() && (xml.name() == QLatin1String("person") || xml.name() == QLatin1String("user"))) {
            break;
        }
    }

    if (!hasAvatarPic) {
        person.setAvatarUrl(QUrl());
    }

    return person;
}

QStringList Person::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("person")) << QStringLiteral("user");
}
