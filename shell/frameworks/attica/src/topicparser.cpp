/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "topicparser.h"
#include "atticautils.h"

using namespace Attica;

Topic Topic::Parser::parseXml(QXmlStreamReader &xml)
{
    Topic topic;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                topic.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("forumId")) {
                topic.setForumId(xml.readElementText());
            } else if (xml.name() == QLatin1String("user")) {
                topic.setUser(xml.readElementText());
            } else if (xml.name() == QLatin1String("date")) {
                topic.setDate(Utils::parseQtDateTimeIso8601(xml.readElementText()));
            } else if (xml.name() == QLatin1String("subject")) {
                topic.setSubject(xml.readElementText());
            } else if (xml.name() == QLatin1String("content")) {
                topic.setContent(xml.readElementText());
            } else if (xml.name() == QLatin1String("comments")) {
                topic.setComments(xml.readElementText().toInt());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("topic")) {
            break;
        }
    }

    return topic;
}

QStringList Topic::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("topic"));
}
