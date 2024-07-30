/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "remoteaccountparser.h"
#include <qdebug.h>

using namespace Attica;

RemoteAccount RemoteAccount::Parser::parseXml(QXmlStreamReader &xml)
{
    RemoteAccount remoteaccount;

    // For specs about the XML provided, see here:
    // http://www.freedesktop.org/wiki/Specifications/open-collaboration-services-draft#RemoteAccounts
    while (!xml.atEnd()) {
        // qCDebug(ATTICA) << "XML returned:" << xml.text().toString();
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                remoteaccount.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("type")) {
                remoteaccount.setType(xml.readElementText());
            } else if (xml.name() == QLatin1String("typeid")) { // FIXME: change to remoteserviceid sometime soon (OCS API change pending
                remoteaccount.setRemoteServiceId(xml.readElementText());
            } else if (xml.name() == QLatin1String("data")) {
                remoteaccount.setData(xml.readElementText());
            } else if (xml.name() == QLatin1String("login")) {
                remoteaccount.setLogin(xml.readElementText());
            } else if (xml.name() == QLatin1String("password")) {
                remoteaccount.setPassword(xml.readElementText());
            }
        } else if (xml.isEndElement() && (xml.name() == QLatin1String("remoteaccount") || xml.name() == QLatin1String("user"))) {
            break;
        }
    }
    return remoteaccount;
}

QStringList RemoteAccount::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("remoteaccount")) << QStringLiteral("user");
}
