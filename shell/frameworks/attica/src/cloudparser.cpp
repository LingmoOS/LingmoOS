/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2012 Laszlo Papp <lpapp@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "cloudparser.h"
#include "atticautils.h"

using namespace Attica;

Cloud Cloud::Parser::parseXml(QXmlStreamReader &xml)
{
    Cloud cloud;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("name")) {
                cloud.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("url")) {
                cloud.setUrl(xml.readElementText());
                // TODO: there should be usage for the attica icon class
            } else if (xml.name() == QLatin1String("icon")) {
                cloud.setIcon(QUrl(xml.readElementText()));
            } else if (xml.name() == QLatin1String("quota")) {
                cloud.setQuota(xml.readElementText().toULongLong());
            } else if (xml.name() == QLatin1String("free")) {
                cloud.setFree(xml.readElementText().toULongLong());
            } else if (xml.name() == QLatin1String("used")) {
                cloud.setUsed(xml.readElementText().toULongLong());
            } else if (xml.name() == QLatin1String("relative")) {
                cloud.setRelative(xml.readElementText().toFloat());
            } else if (xml.name() == QLatin1String("key")) {
                cloud.setKey(xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("cloud")) {
            break;
        }
    }

    return cloud;
}

QStringList Cloud::Parser::xmlElement() const
{
    return QStringList(QLatin1String("cloud"));
}
