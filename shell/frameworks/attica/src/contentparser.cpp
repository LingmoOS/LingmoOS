/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "contentparser.h"

#include <QDateTime>
#include <QDebug>

using namespace Attica;

Content Content::Parser::parseXml(QXmlStreamReader &xml)
{
    Content content;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                content.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                content.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("score")) {
                content.setRating(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("downloads")) {
                content.setDownloads(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("comments")) {
                content.setNumberOfComments(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("created")) {
                // Qt doesn't accept +-Timezone modifiers, truncate if the string contains them
                QString dateString = xml.readElementText().left(19);
                content.setCreated(QDateTime::fromString(dateString, Qt::ISODate));
            } else if (xml.name() == QLatin1String("changed")) {
                // Qt doesn't accept +-Timezone modifiers, truncate if the string contains them
                QString dateString = xml.readElementText().left(19);
                content.setUpdated(QDateTime::fromString(dateString, Qt::ISODate));
            } else if (xml.name() == QLatin1String("icon")) {
                Icon icon;
                icon.setUrl(QUrl(xml.readElementText()));

                const QXmlStreamAttributes attributes = xml.attributes();

                const auto width = attributes.value(QLatin1String("width"));
                if (!width.isEmpty()) {
                    icon.setWidth(width.toInt());
                }

                const auto height = attributes.value(QLatin1String("height"));
                if (!height.isEmpty()) {
                    icon.setHeight(height.toInt());
                }

                // append the icon to the current list of icons
                QList<Icon> icons;
                icons = content.icons();
                icons.append(icon);
                content.setIcons(icons);
            } else if (xml.name() == QLatin1String("video")) {
                QUrl video(xml.readElementText());
                // append the video to the current list of videos
                QList<QUrl> videos;
                videos = content.videos();
                videos.append(video);
                content.setVideos(videos);
            } else if (xml.name() == QLatin1String("tags")) {
                content.setTags(xml.readElementText().split(QLatin1Char(',')));
            } else {
                content.addAttribute(xml.name().toString(), xml.readElementText());
            }
        }

        if (xml.isEndElement() && xml.name() == QLatin1String("content")) {
            break;
        }
    }

    // in case the server only sets creation date, use that as updated also
    if (content.updated().isNull()) {
        content.setUpdated(content.created());
    }

    return content;
}

QStringList Content::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("content"));
}
