/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "folderparser.h"

using namespace Attica;

Folder Folder::Parser::parseXml(QXmlStreamReader &xml)
{
    Folder folder;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                folder.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                folder.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("messagecount")) {
                folder.setMessageCount(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("type")) {
                folder.setType(xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("folder")) {
            break;
        }
    }

    return folder;
}

QStringList Folder::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("folder"));
}
