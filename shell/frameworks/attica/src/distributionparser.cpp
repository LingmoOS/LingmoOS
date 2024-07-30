/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "distributionparser.h"

using namespace Attica;

QStringList Distribution::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("distribution"));
}

Distribution Distribution::Parser::parseXml(QXmlStreamReader &xml)
{
    Distribution item;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                item.setId(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("name")) {
                item.setName(xml.readElementText());
            }
        }
        if (xml.isEndElement() && xml.name() == QLatin1String("distribution")) {
            break;
        }
    }
    return item;
}
