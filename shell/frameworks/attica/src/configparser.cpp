/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "configparser.h"

using namespace Attica;

Config Config::Parser::parseXml(QXmlStreamReader &xml)
{
    Config config;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("version")) {
                config.setVersion(xml.readElementText());
            } else if (xml.name() == QLatin1String("website")) {
                config.setWebsite(xml.readElementText());
            } else if (xml.name() == QLatin1String("host")) {
                config.setHost(xml.readElementText());
            } else if (xml.name() == QLatin1String("contact")) {
                config.setContact(xml.readElementText());
            } else if (xml.name() == QLatin1String("ssl")) {
                config.setSsl(xml.readElementText() == QLatin1String("true"));
            }
        }

        if (xml.isEndElement() && xml.name() == QLatin1String("data")) {
            break;
        }
    }

    return config;
}

QStringList Config::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("data"));
}
