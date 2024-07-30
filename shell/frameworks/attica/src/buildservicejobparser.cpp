/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "buildservicejobparser.h"
#include <qdebug.h>

using namespace Attica;

BuildServiceJob BuildServiceJob::Parser::parseXml(QXmlStreamReader &xml)
{
    BuildServiceJob buildservicejob;

    // For specs about the XML provided, see here:
    // http://www.freedesktop.org/wiki/Specifications/open-collaboration-services-draft#BuildServiceJobs
    while (!xml.atEnd()) {
        // qCDebug(ATTICA) << "XML returned:" << xml.text().toString();
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) { // FIXME: server should give "id" here ...
                buildservicejob.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("project")) {
                buildservicejob.setProjectId(xml.readElementText());
            } else if (xml.name() == QLatin1String("buildservice")) {
                buildservicejob.setBuildServiceId(xml.readElementText());
            } else if (xml.name() == QLatin1String("target")) {
                buildservicejob.setTarget(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                buildservicejob.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("status")) {
                int status = xml.readElementText().toInt();
                buildservicejob.setStatus(status);
            } else if (xml.name() == QLatin1String("progress")) {
                qreal progress = (qreal)(xml.readElementText().toFloat());
                buildservicejob.setProgress(progress);
            } else if (xml.name() == QLatin1String("message")) {
                buildservicejob.setMessage(xml.readElementText());
            } else if (xml.name() == QLatin1String("url")) {
                buildservicejob.setUrl(xml.readElementText());
            }
        } else if (xml.isEndElement() && (xml.name() == QLatin1String("buildjob") || xml.name() == QLatin1String("user"))) {
            break;
        }
    }
    return buildservicejob;
}

QStringList BuildServiceJob::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("buildjob")) << QStringLiteral("user");
}
