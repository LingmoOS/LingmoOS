/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "projectparser.h"
#include <qdebug.h>

using namespace Attica;

Project Project::Parser::parseXml(QXmlStreamReader &xml)
{
    Project project;

    // For specs about the XML provided, see here:
    // http://www.freedesktop.org/wiki/Specifications/open-collaboration-services-draft#Projects
    while (!xml.atEnd()) {
        // qCDebug(ATTICA) << "XML returned:" << xml.text().toString();
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("projectid")) {
                project.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                project.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("version")) {
                project.setVersion(xml.readElementText());
            } else if (xml.name() == QLatin1String("license")) {
                project.setLicense(xml.readElementText());
            } else if (xml.name() == QLatin1String("url")) {
                project.setUrl(xml.readElementText());
            } else if (xml.name() == QLatin1String("summary")) {
                project.setSummary(xml.readElementText());
            } else if (xml.name() == QLatin1String("description")) {
                project.setDescription(xml.readElementText());
            } else if (xml.name() == QLatin1String("specfile")) {
                project.setSpecFile(xml.readElementText());
            } else if (xml.name() == QLatin1String("developers")) {
                project.setDevelopers(xml.readElementText().split(QLatin1Char('\n')));
            } else if (xml.name() == QLatin1String("projectlist")) {
                QXmlStreamReader list_xml(xml.readElementText());
                while (!list_xml.atEnd()) {
                    list_xml.readNext();
                    if (xml.name() == QLatin1String("projectid")) {
                        project.setSpecFile(xml.readElementText());
                    }
                }
            }
        } else if (xml.isEndElement() && (xml.name() == QLatin1String("project") || xml.name() == QLatin1String("user"))) {
            break;
        }
    }
    return project;
}

QStringList Project::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("project")) << QStringLiteral("user");
}
