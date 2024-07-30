/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "commentparser.h"
#include "atticautils.h"
#include <QDebug>

using namespace Attica;

Comment Comment::Parser::parseXml(QXmlStreamReader &xml)
{
    Comment comment;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                comment.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("subject")) {
                comment.setSubject(xml.readElementText());
            } else if (xml.name() == QLatin1String("text")) {
                comment.setText(xml.readElementText());
            } else if (xml.name() == QLatin1String("childcount")) {
                comment.setChildCount(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("user")) {
                comment.setUser(xml.readElementText());
            } else if (xml.name() == QLatin1String("date")) {
                comment.setDate(Utils::parseQtDateTimeIso8601(xml.readElementText()));
            } else if (xml.name() == QLatin1String("score")) {
                comment.setScore(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("children")) {
                // This may seem strange, however we are dealing with a situation where we may
                // receive multiple children subsections (the standard accepts this, and certain
                // server implementations do do this)
                QList<Comment> children = comment.children();
                children += parseXmlChildren(xml);
                comment.setChildren(children);
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("comment")) {
            break;
        }
    }

    return comment;
}

QList<Comment> Comment::Parser::parseXmlChildren(QXmlStreamReader &xml)
{
    QList<Comment> children;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("comment")) {
                Comment comment = parseXml(xml);
                children.append(comment);
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("children")) {
            break;
        }
    }

    return children;
}

QStringList Comment::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("comment"));
}
