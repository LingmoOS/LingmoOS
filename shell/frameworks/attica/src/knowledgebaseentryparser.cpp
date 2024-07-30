/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "knowledgebaseentryparser.h"

using namespace Attica;

KnowledgeBaseEntry KnowledgeBaseEntry::Parser::parseXml(QXmlStreamReader &xml)
{
    KnowledgeBaseEntry knowledgeBase;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                knowledgeBase.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("status")) {
                knowledgeBase.setStatus(xml.readElementText());
            } else if (xml.name() == QLatin1String("contentId")) {
                knowledgeBase.setContentId(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("user")) {
                knowledgeBase.setUser(xml.readElementText());
            } else if (xml.name() == QLatin1String("changed")) {
                knowledgeBase.setChanged(QDateTime::fromString(xml.readElementText(), Qt::ISODate));
            } else if (xml.name() == QLatin1String("description")) {
                knowledgeBase.setDescription(xml.readElementText());
            } else if (xml.name() == QLatin1String("answer")) {
                knowledgeBase.setAnswer(xml.readElementText());
            } else if (xml.name() == QLatin1String("comments")) {
                knowledgeBase.setComments(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("detailpage")) {
                knowledgeBase.setDetailPage(QUrl(xml.readElementText()));
            } else if (xml.name() == QLatin1String("contentid")) {
                knowledgeBase.setContentId(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("name")) {
                knowledgeBase.setName(xml.readElementText());
            } else {
                knowledgeBase.addExtendedAttribute(xml.name().toString(), xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("content")) {
            break;
        }
    }

    return knowledgeBase;
}

QStringList KnowledgeBaseEntry::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("content"));
}
