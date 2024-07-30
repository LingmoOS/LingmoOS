/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "licenseparser.h"

using namespace Attica;

QStringList License::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("license"));
}

License License::Parser::parseXml(QXmlStreamReader &xml)
{
    License item;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                item.setId(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("name")) {
                item.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("link")) {
                item.setUrl(QUrl(xml.readElementText()));
            }
        }
        if (xml.isEndElement() && xml.name() == QLatin1String("license")) {
            break;
        }
    }
    return item;
}
