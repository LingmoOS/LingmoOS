/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "forumparser.h"
#include "atticautils.h"

using namespace Attica;

Forum Forum::Parser::parseXml(QXmlStreamReader &xml)
{
    Forum forum;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                forum.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                forum.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("description")) {
                forum.setDescription(xml.readElementText());
            } else if (xml.name() == QLatin1String("date")) {
                forum.setDate(Utils::parseQtDateTimeIso8601(xml.readElementText()));
            } else if (xml.name() == QLatin1String("icon")) {
                forum.setIcon(QUrl(xml.readElementText()));
            } else if (xml.name() == QLatin1String("childcount")) {
                forum.setChildCount(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("children")) {
                QList<Forum> children = parseXmlChildren(xml);
                forum.setChildren(children);
            } else if (xml.name() == QLatin1String("topics")) {
                forum.setTopics(xml.readElementText().toInt());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("forum")) {
            break;
        }
    }

    return forum;
}

QList<Forum> Forum::Parser::parseXmlChildren(QXmlStreamReader &xml)
{
    QList<Forum> children;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("forum")) {
                Forum forum = parseXml(xml);
                children.append(forum);
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("children")) {
            break;
        }
    }

    return children;
}

QStringList Forum::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("forum"));
}
