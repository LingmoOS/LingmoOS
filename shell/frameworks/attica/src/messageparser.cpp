/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "messageparser.h"

using namespace Attica;

Message Message::Parser::parseXml(QXmlStreamReader &xml)
{
    Message message;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                message.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("messagefrom")) {
                message.setFrom(xml.readElementText());
            } else if (xml.name() == QLatin1String("messageto")) {
                message.setTo(xml.readElementText());
            } else if (xml.name() == QLatin1String("senddate")) {
                message.setSent(QDateTime::fromString(xml.readElementText(), Qt::ISODate));
            } else if (xml.name() == QLatin1String("status")) {
                message.setStatus(Message::Status(xml.readElementText().toInt()));
            } else if (xml.name() == QLatin1String("subject")) {
                message.setSubject(xml.readElementText());
            } else if (xml.name() == QLatin1String("body")) {
                message.setBody(xml.readElementText());
            }
        }

        if (xml.isEndElement() && xml.name() == QLatin1String("message")) {
            break;
        }
    }

    return message;
}

QStringList Message::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("message"));
}
