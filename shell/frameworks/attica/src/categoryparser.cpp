/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "categoryparser.h"

using namespace Attica;

Category Category::Parser::parseXml(QXmlStreamReader &xml)
{
    Category category;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                category.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                category.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("display_name")) {
                category.setDisplayName(xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("category")) {
            break;
        }
    }

    return category;
}

QStringList Category::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("category"));
}
