/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "buildservicejoboutputparser.h"
#include "qdebug.h"

using namespace Attica;

BuildServiceJobOutput BuildServiceJobOutput::Parser::parseXml(QXmlStreamReader &xml)
{
    BuildServiceJobOutput buildservicejoboutput;

    // For specs about the XML provided, see here:
    // http://www.freedesktop.org/wiki/Specifications/open-collaboration-services-draft#BuildServiceJobs
    while (!xml.atEnd()) {
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("output")) {
                buildservicejoboutput.setOutput(xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("output")) {
            break;
        }
        xml.readNext();
    }
    return buildservicejoboutput;
}

QStringList BuildServiceJobOutput::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("output"));
}
