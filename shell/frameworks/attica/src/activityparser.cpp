/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "activityparser.h"

#include <QDateTime>
#include <QRegularExpression>

using namespace Attica;

Activity Activity::Parser::parseXml(QXmlStreamReader &xml)
{
    Activity activity;
    Person person;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                activity.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("personid")) {
                person.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("avatarpic")) {
                person.setAvatarUrl(QUrl(xml.readElementText()));
            } else if (xml.name() == QLatin1String("firstname")) {
                person.setFirstName(xml.readElementText());
            } else if (xml.name() == QLatin1String("lastname")) {
                person.setLastName(xml.readElementText());
            } else if (xml.name() == QLatin1String("timestamp")) {
                QString timestampString = xml.readElementText();
                timestampString.remove(QRegularExpression(QStringLiteral("\\+.*$")));
                QDateTime timestamp = QDateTime::fromString(timestampString, Qt::ISODate);
                activity.setTimestamp(timestamp);
            } else if (xml.name() == QLatin1String("message")) {
                activity.setMessage(xml.readElementText());
            } else if (xml.name() == QLatin1String("link")) {
                activity.setLink(QUrl(xml.readElementText()));
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("activity")) {
            break;
        }
    }

    activity.setAssociatedPerson(person);
    return activity;
}

QStringList Activity::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("activity"));
}
