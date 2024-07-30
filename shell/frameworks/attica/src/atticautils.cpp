/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "atticautils.h"
#include <QStringList>
#include <QTimeZone>

using namespace Attica;

QDateTime Utils::parseQtDateTimeIso8601(const QString &str)
{
    QDateTime result;
    QStringList list;
    QString datetime;

    int tzsign = 0;
    if (str.indexOf(QLatin1String("+")) != -1) {
        list = str.split(QStringLiteral("+"));
        datetime = list[0];
        tzsign = 1;
    } else if (str.indexOf(QLatin1String("-")) != -1) {
        list = str.split(QStringLiteral("-"));
        datetime = list[0];
        tzsign = -1;
    } else {
        datetime = str;
    }

    // parse date time
    result = QDateTime::fromString(datetime, Qt::ISODate);
    result.setTimeZone(QTimeZone::utc());

    // parse timezone
    if (list.count() == 2) {
        QString tz = list[1];
        int hh = 0;
        int mm = 0;
        int tzsecs = 0;
        if (tz.indexOf(QLatin1Char(':')) != -1) {
            QStringList tzlist = tz.split(QLatin1Char(':'));
            if (tzlist.count() == 2) {
                hh = tzlist[0].toInt();
                mm = tzlist[1].toInt();
            }
        } else {
            QStringView sv(tz);
            hh = sv.left(2).toInt();
            mm = sv.mid(2).toInt();
        }

        tzsecs = 60 * 60 * hh + 60 * mm;
        result = result.addSecs(-tzsecs * tzsign);
    }

    return result;
}

const char *Utils::toString(QNetworkAccessManager::Operation operation)
{
    switch (operation) {
    case QNetworkAccessManager::GetOperation:
        return "Get";
    case QNetworkAccessManager::HeadOperation:
        return "Head";
    case QNetworkAccessManager::PutOperation:
        return "Put";
    case QNetworkAccessManager::PostOperation:
        return "Post";
    case QNetworkAccessManager::DeleteOperation:
        return "Delete";
    case QNetworkAccessManager::CustomOperation:
        return "Custom";
    default:
        return "unknown";
    }
    return "invalid";
}
