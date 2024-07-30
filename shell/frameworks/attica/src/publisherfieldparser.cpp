/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "publisherfieldparser.h"

using namespace Attica;

PublisherField PublisherField::Parser::parseXml(QXmlStreamReader &xml)
{
    PublisherField fld;
    while (!xml.atEnd()) {
        xml.readNextStartElement();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("fieldtype")) {
                fld.setType(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                fld.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("data")) {
                fld.setData(xml.readElementText());
            }
        } else if (xml.isEndElement() && (xml.name() == QLatin1String("field"))) {
            xml.readNext();
            break;
        }
    }
    return fld;
}

QStringList PublisherField::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("field"));
}
