/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "eventparser.h"

#include <QRegularExpression>

using namespace Attica;

Event Event::Parser::parseXml(QXmlStreamReader &xml)
{
    Event event;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                event.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                event.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("description")) {
                event.setDescription(xml.readElementText());
            } else if (xml.name() == QLatin1String("user")) {
                event.setUser(xml.readElementText());
            } else if (xml.name() == QLatin1String("startdate")) {
                QString date = xml.readElementText().remove(QRegularExpression(QStringLiteral("\\+.*$")));
                event.setStartDate(QDate::fromString(date, Qt::ISODate));
            } else if (xml.name() == QLatin1String("enddate")) {
                QString date = xml.readElementText().remove(QRegularExpression(QStringLiteral("\\+.*$")));
                event.setEndDate(QDate::fromString(date, Qt::ISODate));
            } else if (xml.name() == QLatin1String("latitude")) {
                event.setLatitude(xml.readElementText().toFloat());
            } else if (xml.name() == QLatin1String("longitude")) {
                event.setLongitude(xml.readElementText().toFloat());
            } else if (xml.name() == QLatin1String("homepage")) {
                event.setHomepage(QUrl(xml.readElementText()));
            } else if (xml.name() == QLatin1String("country")) {
                event.setCountry(xml.readElementText());
            } else if (xml.name() == QLatin1String("city")) {
                event.setCity(xml.readElementText());
            } else {
                event.addExtendedAttribute(xml.name().toString(), xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("event")) {
            break;
        }
    }

    return event;
}

QStringList Event::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("event"));
}
