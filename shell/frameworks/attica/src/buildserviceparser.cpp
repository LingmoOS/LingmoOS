/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "buildserviceparser.h"
#include <qdebug.h>

using namespace Attica;

BuildService BuildService::Parser::parseXml(QXmlStreamReader &xml)
{
    // For specs about the XML provided, see here:
    // http://www.freedesktop.org/wiki/Specifications/open-collaboration-services-draft

    BuildService buildservice;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                buildservice.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                buildservice.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("registrationurl")) {
                buildservice.setUrl(xml.readElementText());
            } else if (xml.name() == QLatin1String("supportedtargets")) {
                while (!xml.atEnd()) {
                    xml.readNextStartElement();
                    if (xml.isStartElement()) {
                        if (xml.name() == QLatin1String("target")) {
                            Target t;
                            while (!xml.atEnd()) {
                                xml.readNextStartElement();
                                if (xml.isStartElement()) {
                                    if (xml.name() == QLatin1String("id")) {
                                        t.id = xml.readElementText();
                                    } else if (xml.name() == QLatin1String("name")) {
                                        t.name = xml.readElementText();
                                    }
                                } else if (xml.isEndElement() && (xml.name() == QLatin1String("target"))) {
                                    xml.readNext();
                                    break;
                                }
                            }
                            buildservice.addTarget(t);
                        }
                    } else if (xml.isEndElement() && (xml.name() == QLatin1String("supportedtargets"))) {
                        xml.readNext();
                        break;
                    }
                }
            }
        } else if (xml.isEndElement() //
                   && (xml.name() == QLatin1String("buildservice") || xml.name() == QLatin1String("user"))) {
            break;
        }
    }
    return buildservice;
}

QStringList BuildService::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("buildservice")) << QStringLiteral("user");
}
