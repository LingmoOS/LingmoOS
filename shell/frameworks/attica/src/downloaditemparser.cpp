/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "downloaditemparser.h"

#include <QXmlStreamReader>

using namespace Attica;

QStringList DownloadItem::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("content"));
}

DownloadItem DownloadItem::Parser::parseXml(QXmlStreamReader &xml)
{
    DownloadItem item;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("downloadlink")) {
                item.setUrl(QUrl(xml.readElementText()));
            } else if (xml.name() == QLatin1String("mimetype")) {
                item.setMimeType(xml.readElementText());
            } else if (xml.name() == QLatin1String("packagename")) {
                item.setPackageName(xml.readElementText());
            } else if (xml.name() == QLatin1String("packagerepository")) {
                item.setPackageRepository(xml.readElementText());
            } else if (xml.name() == QLatin1String("gpgfingerprint")) {
                item.setGpgFingerprint(xml.readElementText());
            } else if (xml.name() == QLatin1String("gpgsignature")) {
                item.setGpgSignature(xml.readElementText());
            } else if (xml.name() == QLatin1String("downloadway")) {
                item.setType(DownloadDescription::Type(xml.readElementText().toInt()));
            }
        }
    }
    return item;
}
